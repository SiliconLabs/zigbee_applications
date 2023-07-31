# Error Interpretation from Capture Traces
 In the trace trace_ERROR_joining_two_devices.isd, we can see the SED is trying to join the network but all transaction is done twice such as Update Device, Tunnel Data, and even the Transport Key (NWK) is send twice, one with APS encrypted and one without.

 To solve this issue : I added the software component "End Device Support" to the project configurator of JoinWithCode_SED.