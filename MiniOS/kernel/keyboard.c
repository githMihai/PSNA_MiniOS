#include "keyboard.h"
#include "screen.h"

static KeyStatus keyState;

static PFUNC func;

void KeyboardInit()
{
	keyState.key = 0;
	keyState.NumLock = FALSE;
	keyState.ScrollLock = FALSE;
	keyState.CapsLock = FALSE;
	keyState.Shift = FALSE;
	keyState.Ctrl = FALSE;
	keyState.Alt = FALSE;
	keyState.Pressed = FALSE;
	keyState.Released = FALSE;
}

/**
 *	\name KeyboardReadStatus
 *	\brief Read Keyboard controller Status Register
 *	\return Value from Status Register
 */
BYTE KeyboardReadStatus()
{
	return __inbyte(KEYBOARD_CTRL_STATUS_REG);
}

void KeyboardCtrlSendCommand(BYTE cmd)
{
	while (1)
	{
		if ((KeyboardReadStatus() & KYBRD_CTRL_STATS_MASK_IN_BUF) == 0)
			break;
	}
	__outbyte(KEYBOARD_CTRL_STATUS_REG, cmd);
}

void KeyboardEncSendCommand(BYTE cmd)
{
	__outbyte(KEYBOARD_ENC_CMD_REG, cmd);
}

WORD ReadScancode()
{
	WORD scancode;
	scancode = __inbyte(KEYBOARD_ENC_DATA_REG);
	return scancode;
}

void KeyboardInterruptHandler()
{
	BYTE status;
	WORD keycode;

	KeyStatus copyState;

	status = __inbyte(KEYBOARD_CTRL_STATUS_REG);
	if (status & 0x01)
	{
		keycode = __inbyte(KEYBOARD_ENC_DATA_REG);
		switch (keycode)
		{
			case KEY_KP_NUMLOCK:	keyState.NumLock = !keyState.NumLock; break;
			case KEY_SCROLLLOCK:	keyState.ScrollLock = !keyState.ScrollLock; break;
			case KEY_CAPSLOCK:		keyState.CapsLock = !keyState.CapsLock; break;
			case KEY_LSHIFT:		keyState.Shift = TRUE; break;
			case KEY_RSHIFT:		keyState.Shift = TRUE; break;
			case KEY_LCTRL:			keyState.Ctrl = TRUE; break;
			case KEY_LALT:			keyState.Alt = TRUE; break;
			case KEY_LSHIFT | 0x80:	keyState.Shift = FALSE; break;
			case KEY_RSHIFT | 0x80:	keyState.Shift = FALSE; break;
			case KEY_LCTRL | 0x80:	keyState.Ctrl = FALSE; break;
			case KEY_LALT | 0x80:	keyState.Alt = FALSE; break;
			case 0xe0:				keyState.e0_received = TRUE; break;
			default:
			{
				keyState.Released = (keycode & 0x80) != 0;
				keyState.Pressed = !keyState.Released;
				keycode &= ~0x80u;
				if (keyState.e0_received)
				{
					keyState.key = (0xe0 << 8) | keycode;
					keyState.e0_received = FALSE;
				}
				else
				{
					keyState.key = keycode;
				}
				copyState = keyState;
				func(copyState);
			}
		}
	}
}

void KeyboardSubscribe(PFUNC subscriptionFunc)
{
	func = subscriptionFunc;
}