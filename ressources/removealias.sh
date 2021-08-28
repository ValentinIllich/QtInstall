#!/bin/bash

osascript <<END_SCRIPT
tell application "Finder"
  delete alias file "$1" of desktop 
end tell
END_SCRIPT
