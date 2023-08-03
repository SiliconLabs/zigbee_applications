/***************************************************************************//**
 * @file human_detection_ai.cc
 * @brief human detection ai file.
 *******************************************************************************
 * # License
 * <b>Copyright 2022 Silicon Laboratories Inc. www.silabs.com</b>
 *******************************************************************************
 *
 * SPDX-License-Identifier: Zlib
 *
 * The licensor of this software is Silicon Laboratories Inc.
 *
 * This software is provided \'as-is\', without any express or implied
 * warranty. In no event will the authors be held liable for any damages
 * arising from the use of this software.
 *
 * Permission is granted to anyone to use this software for any purpose,
 * including commercial applications, and to alter it and redistribute it
 * freely, subject to the following restrictions:
 *
 * 1. The origin of this software must not be misrepresented{} you must not
 *    claim that you wrote the original software. If you use this software
 *    in a product, an acknowledgment in the product documentation would be
 *    appreciated but is not required.
 * 2. Altered source versions must be plainly marked as such, and must not be
 *    misrepresented as being the original software.
 * 3. This notice may not be removed or altered from any source distribution.
 *
 *******************************************************************************
 * # Experimental Quality
 * This code has not been formally tested and is provided as-is. It is not
 * suitable for production environments. In addition, this code will not be
 * maintained and there may be no bug maintenance planned for these resources.
 * Silicon Labs may update projects from time to time.
 ******************************************************************************/
#include <math.h>

#include "sl_sleeptimer.h"

#include "tensorflow/lite/micro/tflite_bridge/micro_error_reporter.h"
#include "tensorflow/lite/micro/micro_interpreter.h"
#include "tensorflow/lite/micro/micro_mutable_op_resolver.h"
#include "tensorflow/lite/schema/schema_generated.h"

#include "sl_tflite_micro_model_parameters.h"
#include "sl_tflite_micro_init.h"

#include "printf.h"

#include "human_detection_ai.h"
#include "image_handler.h"
#include "recognize_commands.h"
#include "human_detection_ui.h"

const char *category_labels[TFLITE_MODEL_CLASS_COUNT] = SL_TFLITE_MODEL_CLASSES;

//  Instance Pointers
static RecognizeCommands *command_recognizer = nullptr;

app_settings_t app_settings = {
#ifdef SL_TFLITE_MODEL_SAMPLEWISE_NORM_RESCALE
    .samplewise_norm_rescale = SL_TFLITE_MODEL_SAMPLEWISE_NORM_RESCALE,
#else
    .samplewise_norm_rescale = 0.0f,
#endif
#ifdef SL_TFLITE_MODEL_SAMPLEWISE_NORM_MEAN_AND_STD
    .samplewise_norm_mean_and_std = SL_TFLITE_MODEL_SAMPLEWISE_NORM_MEAN_AND_STD,
#else
    .samplewise_norm_mean_and_std = false,
#endif
    .verbose_inference_output = false,
    .average_window_duration_ms = 1000,
    .minimum_count = 1,
    .detection_threshold = 160,
    .suppression_count = 1,
};

static void standardize_image_data(float *image_data, uint32_t image_size);

sl_status_t human_detection_ai_init(void) {

  // Initialize ir sensor
  sl_status_t setup_status = mlx90640_setup();
  if (setup_status != SL_STATUS_OK) {
    printf("Set up mlx90640 failed\n");
    return setup_status;
  }

  // Instantiate CommandRecognizer
  static RecognizeCommands static_recognizer(
      sl_tflite_micro_get_error_reporter(),
      app_settings.average_window_duration_ms,
      app_settings.detection_threshold,
      app_settings.suppression_count,
      app_settings.minimum_count
      );
  command_recognizer = &static_recognizer;

  printf("Start recognition.\n");
  return SL_STATUS_OK;
}

void human_detection_ai_loop(void)
{
  uint8_t found_command_index = 0;
  uint8_t score = 0;
  bool is_new_command = false;
  uint32_t current_time_stamp;
  float *image_data;
  int image_size;

  // Retrieve the image from the camera
  sl_status_t img_status = mlx90640_read_image(&image_data, &image_size);
  if (img_status != SL_STATUS_OK) {
    return;
  }

  // Standardize the image data (if necessary)
  // and copy to model input tensor
  standardize_image_data(image_data, image_size);

  // Run the model on the spectrogram input and make sure it succeeds.
  TfLiteStatus invoke_status = sl_tflite_micro_get_interpreter()->Invoke();
  if (invoke_status != kTfLiteOk) {
    printf("Invoke failed\n");
    return;
  }

  // Determine whether a command was recognized based on the output of inference
  // Get current time stamp needed by CommandRecognizer
  current_time_stamp = sl_sleeptimer_tick_to_ms(sl_sleeptimer_get_tick_count());

  TfLiteStatus process_status = command_recognizer->ProcessLatestResults(
                                sl_tflite_micro_get_output_tensor(),
                                current_time_stamp,
                                &found_command_index,
                                &score,
                                &is_new_command);
  if (process_status != kTfLiteOk) {
    printf("RecognizeCommands::ProcessLatestResults() failed\n");
    return;
  }

  const char *label = category_labels[found_command_index];

  if (is_new_command) {
    human_detection_ui_predict_update(found_command_index, score);
    human_detection_ai_get_index(found_command_index);
    printf("Detected class=%d label=%s score=%d @%ldms\n",
           found_command_index,
           label,
           score,
           current_time_stamp);
  }
}

/***************************************************************************//**
 * @brief Normalize the source buffer by mean and STD
 *
 * dst_float32 = (src - mean(src)) / std(src)
 ******************************************************************************/
template<typename SrcType>
void samplewise_mean_std_tensor(const SrcType *src, float *dst, uint32_t length)
{
  float mean = 0.0f;
  float count = 0.0f;
  float m2 = 0.0f;
  const SrcType *ptr;

    // Calculate the STD and mean
  ptr = src;
  for (int i = length; i > 0; --i) {
    const float value = (float)(*ptr++);

    count += 1;

    const float delta = value - mean;
    mean += delta / count;
    const float delta2 = value - mean;
    m2 += delta * delta2;
  }

  const float variance = m2 / count;
  const float std = sqrtf(variance);
  const float std_recip = 1.0f / std; // multiplication is faster than division

  // Subtract the mean and divide by the STD
  ptr = src;
  for (int i = length; i > 0; --i) {
    const float value = (float)(*ptr++);
    const float x = value - mean;

    *dst++ = x * std_recip;
  }
}

/***************************************************************************//**
 * @brief Scale the source buffer by the given scaler
 *
 * dst_float32 = src * scaler
 ******************************************************************************/
template<typename SrcType>
void scale_tensor(float scaler, const SrcType *src, float *dst, uint32_t length)
{
    for(; length > 0; --length)
    {
        const float src_flt = static_cast<SrcType>(*src++);
        *dst++ = src_flt * scaler;
    }
}

/***************************************************************************//**
 * Standardize the image and copy to model input tensor
 ******************************************************************************/
static void standardize_image_data(float *image_data, uint32_t image_size)
{
  if(app_settings.samplewise_norm_rescale != 0) {
    scale_tensor(app_settings.samplewise_norm_rescale,
                 image_data,
                 sl_tflite_micro_get_input_tensor()->data.f,
                 image_size);
  } else if(app_settings.samplewise_norm_mean_and_std) {
    // input_tensor = (image_data - mean(image_data)) / std(image_data)
    samplewise_mean_std_tensor(image_data,
                               sl_tflite_micro_get_input_tensor()->data.f,
                               image_size);
  }
}
