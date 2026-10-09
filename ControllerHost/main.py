import os
import json
import urllib.error
import urllib.request

# Must be set before pygame.init().
os.environ["SDL_JOYSTICK_ALLOW_BACKGROUND_EVENTS"] = "1"

import pygame
import time

pygame.init()
pygame.joystick.init()

Controller = None
SERVER_URL = os.getenv("CONTROLLER_SERVER_URL")
POST_INTERVAL_SECONDS = 0.02


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

    for Name, Value in Decoded.items():
        print(f"{Name} = {Value}")

    return Decoded


def SendControllerState(decoded):
    request = urllib.request.Request(
        SERVER_URL,
        data=json.dumps(decoded).encode("utf-8"),
        headers={"Content-Type": "application/json"},
        method="POST",
    )

    try:
        with urllib.request.urlopen(request, timeout=0.5) as response:
            if response.status != 204:
                print(f"Server rejected controller state: {response.status}")
    except urllib.error.URLError as error:
        print(f"Could not send controller state: {error.reason}")



try:
    if not SERVER_URL:
        raise RuntimeError("Set CONTROLLER_SERVER_URL to the trusted backend /controller URL")

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
