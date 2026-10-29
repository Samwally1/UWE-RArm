# Backend file map

| File | Responsibility |
| --- | --- |
| `main.py` | HTTP server entry point on port 8000 |
| `api/handlers.py` | Existing GET/POST handlers and process-local arm/controller state |
| `models/robot.py` | `Angles` and `Coordinates` data classes |
| `motion/kinematics.py` | `ForwardKim` and `InverseKim` calculations |
| `motion/packets.py` | Path interpolation and timestamped packet formatting |
| `Dockerfile` | Packages all backend Python modules |

Run `python main.py` from this directory, or use the existing Docker Compose configuration.
Function names, calculations, routes and response formats are unchanged by the regrouping.
