import pygame
import time

pygame.init()
pygame.joystick.init()

def GetController():
    for index in range(pygame.joystick.get_count()):
        controller = pygame.joystick.Joystick(index)

        if "xbox" in controller.get_name().lower():
            controller.init()
            return controller

    return None


def ControllerConnected():
    pygame.event.pump()
    return GetController() is not None

def ReadController():
    pygame.event.pump()
    controller = GetController()

    if controller is None:
        return None

    return {
        "axes": [
            controller.get_axis(i)
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


while True:
    if ControllerConnected():
        print("Controller connected")
        print("-----------------------------------------")
        while ControllerConnected:
            print(ReadController())

    else:
        print("pls Connect Controller")

    time.sleep(1)
