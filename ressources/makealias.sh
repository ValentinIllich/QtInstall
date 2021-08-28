#!/bin/bash

osascript <<END_SCRIPT
tell application "Finder"
   make new alias with properties {name:"$1"} to $type (posix file "$2") at desktop
end tell

END_SCRIPT
