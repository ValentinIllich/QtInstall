#!/bin/bash

osascript <<END_SCRIPT

tell application "System Events"
  make login item at end with properties {path:"$2",name:"$1"}
end tell

END_SCRIPT
