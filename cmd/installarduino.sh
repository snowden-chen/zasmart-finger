#!/bin/bash

if test -d $HOME/zadiy-factory/platform/arduino/arduino-esp32; then
  echo "Arduino Directory exists"
else
  echo "Arduino Directory does not exist"
  exit 1
fi

if test -L $HOME/zadiy-factory/src/components/arduino; then
  echo "Arduino Link exists"
else
  echo "Arduino Link does not exist , creat it."
  ln -s $HOME/zadiy-factory/platform/arduino/arduino-esp32 $HOME/zadiy-factory/src/components/arduino
fi


