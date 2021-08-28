#!/bin/bash

osascript <<END_SCRIPT

tell application "System Events"
	delete login item "$1"
end tell

END_SCRIPT
