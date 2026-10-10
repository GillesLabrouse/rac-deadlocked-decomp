# Native Windows controller diagnostic

Build from the repository root:

```powershell
powershell -ExecutionPolicy Bypass -File tools/check-native.ps1
```

Connect an XInput-compatible controller and run `run-controller-probe.cmd`.
The default records ten samples, one second apart, and keeps the console open.
Press, hold and release buttons to inspect the recovered held/pressed/released
masks. The same input record and callback context survive between samples;
disconnecting a controller requires configuration again when it reconnects.
For a chosen duration, use:

```powershell
.\run-controller-probe.cmd --frames 30
```

The diagnostic uses real Windows `XInputGetState` and `XInputGetCapabilities`,
then executes the recovered Dreadzone configuration/polling state machine,
callback and packet decoder. The backend supports four XInput user indices,
bound as `index = 2 * slot + port`, with both logical fields in `0..1`.
These interfaces and limits follow the [Microsoft XInput reference](https://learn.microsoft.com/en-us/windows/win32/xinput/xinput-game-controller-apis-portal).
It does not run the actor update or game loop. The separate button-repeat pass
requires game state binding and is not installed by this diagnostic.

The host adapter maps A/B/X/Y to Cross/Circle/Square/Triangle, shoulders to L1/R1,
Back/Start to Select/Start, and triggers above the XInput threshold to L2/R2.
Stick bytes retain the full input range, have neutral value 128 and invert Y;
the recovered decoder applies its own deadzone. Digital pressure channels are
adapted to 0/255, while trigger magnitudes remain analog. This is an explicit
Windows packet adaptation, rather than a recovered Sony controller transport.
The inputs are defined by [Microsoft's XINPUT_GAMEPAD contract](https://learn.microsoft.com/en-us/windows/win32/api/xinput/ns-xinput-xinput_gamepad).

No vibration is sent by the diagnostic. Feedback is disabled by default in
the backend; its optional implementation calls Windows `XInputSetState` only
when explicitly enabled and the device/configuration supports the requested
roles. Configuration state is local to the adapter and checked against device
capabilities. Unsupported slots, missing devices and OS errors are reported.

The 223 fixture checks validate conversion boundaries, configuration, actual
callback/decoder press/hold/release behavior and failure handling. Feedback
calls in those tests use an injected OS fixture. The live probe on the current
machine returned `1167` (device not connected) for state and capabilities on
all four indices; attached hardware input has not yet been verified.
