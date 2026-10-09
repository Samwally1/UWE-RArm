import os

# Must be set before pygame.init().
os.environ["SDL_JOYSTICK_ALLOW_BACKGROUND_EVENTS"] = "1"

import pygame
import time

pygame.init()
pygame.joystick.init()

Controller = None


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
    



try:
    while True:

        if ControllerConnected():
            DecodeEvents(ReadController())
        else:
            print("Please connect Xbox controller")
            time.sleep(1)

        time.sleep(0.1)

except KeyboardInterrupt:
    pass

finally:
    pygame.quit()