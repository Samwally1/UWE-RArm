"""Send target (200, 200, 200, 0) at 20 mm/s to the robot backend."""

import sys
import urllib.error
import urllib.request


TARGET_URL = "https://rarm.sjw-home.xyz/target"
TARGET_HEADERS = {"X": "200", "Y": "200", "Z": "200", "A": "0", "Speed": "20"}


def main():
    request = urllib.request.Request(
        TARGET_URL, data=b"", headers=TARGET_HEADERS, method="POST"
    )
    try:
        with urllib.request.urlopen(request, timeout=10) as response:
            if response.status != 204:
                print(f"Unexpected backend response: HTTP {response.status}", file=sys.stderr)
                return 1
    except urllib.error.HTTPError as error:
        detail = error.read().decode("utf-8", errors="replace")
        print(f"Target rejected: HTTP {error.code}\n{detail}", file=sys.stderr)
        return 1
    except (urllib.error.URLError, TimeoutError, OSError) as error:
        print(f"Could not send target: {error}", file=sys.stderr)
        return 1

    print("Target (200, 200, 200, 0) at 20 mm/s accepted (HTTP 204).")
    return 0


if __name__ == "__main__":
    sys.exit(main())
