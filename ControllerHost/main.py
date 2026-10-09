import os
import json
import queue
import threading
import urllib.error
import urllib.request

# Must be set before pygame.init().
os.environ["SDL_JOYSTICK_ALLOW_BACKGROUND_EVENTS"] = "1"

import pygame
import time

pygame.init()
pygame.joystick.init()

Controller = None
SERVER_URL = "https://rarm.sjw-home.xyz/controller"
STATUS_URL = SERVER_URL.rsplit("/controller", 1)[0] + "/status"
POST_INTERVAL_SECONDS = 0.02
StateBuffer = queue.Queue(maxsize=1)


def GetController():
    global Controller

    if Controller is None:
        for index in range(pygame.joystick.get_count()):
            device = pygame.joystick.Joystick(index)

            if "xbox" in device.get_name().lower():
                Controller = device
                print("Connected:", Controller.get_name())
                break

    return Controller


def ControllerConnected():
    return GetController() is not None


def ReadController():
    controller = GetController()

    if controller is None:
        return None

    pygame.event.pump()

    return {
        "axes": [
            round(controller.get_axis(i), 3)
            for i in range(controller.get_numaxes())
        ],
        "buttons": [
            bool(controller.get_button(i))
            for i in range(controller.get_numbuttons())
        ],
        "hats": [
            controller.get_hat(i)
            for i in range(controller.get_numhats())
        ],
    }


def DecodeEvents(Output):

    Decoded = {
        "LSX": Output['axes'][0],
        "LSY": Output['axes'][1],
        "RSX": Output['axes'][2],
        "RSY": Output['axes'][3],
        "A": Output['buttons'][0],
    }

    return Decoded


def SendControllerState(decoded):
    try:
        StateBuffer.get_nowait()
    except queue.Empty:
        pass

    try:
        StateBuffer.put_nowait(decoded)
    except queue.Full:
        pass


def SendBufferedControllerState():
    lastErrorTime = 0.0

    while True:
        decoded = StateBuffer.get()
        request = urllib.request.Request(
            SERVER_URL,
            data=json.dumps(decoded).encode("utf-8"),
            headers={
                "Content-Type": "application/json",
                "User-Agent": "ControllerHost/1.0",
            },
            method="POST",
        )

        try:
            with urllib.request.urlopen(request, timeout=0.5) as response:
                if response.status != 204:
                    print(f"Server rejected controller state: {response.status}")
        except (urllib.error.URLError, TimeoutError, OSError) as error:
            if time.monotonic() - lastErrorTime >= 1:
                print(f"Could not send controller state: {error}")
                lastErrorTime = time.monotonic()
        finally:
            StateBuffer.task_done()


def PrintEspStatus():
    lastStatus = None

    while True:
        try:
            request = urllib.request.Request(
                STATUS_URL,
                headers={"User-Agent": "ControllerHost/1.0"},
            )
            with urllib.request.urlopen(request, timeout=0.5) as response:
                status = response.read().decode("utf-8")

            if status != lastStatus:
                print(f"Arm status: {status}")
                lastStatus = status
        except (urllib.error.URLError, TimeoutError, OSError, json.JSONDecodeError) as error:
            print(f"Could not read ESP status: {error}")

        time.sleep(0.2)



try:
    if not SERVER_URL:
        raise RuntimeError("Set CONTROLLER_SERVER_URL to the trusted backend /controller URL")

    print(f"Sending controller state to: {SERVER_URL}")
    threading.Thread(target=SendBufferedControllerState, daemon=True).start()
    threading.Thread(target=PrintEspStatus, daemon=True).start()

    while True:

        if ControllerConnected():
            SendControllerState(DecodeEvents(ReadController()))
        else:
            print("Please connect Xbox controller")
            time.sleep(1)

        time.sleep(POST_INTERVAL_SECONDS)

except KeyboardInterrupt:
    pass

finally:
    pygame.quit()
