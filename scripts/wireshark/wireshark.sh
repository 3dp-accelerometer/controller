#!/usr/bin/env bash

SCRIPT_DIR="$(readlink -f $( cd -- "$(dirname "$0")" >/dev/null 2>&1 ; pwd -P ))"
SCRIPT_NAME="$(basename "$(test -L "$0" && readlink "$0" || echo "$0")")"

VENDOR_ID="1209"
PRODUCT_ID="e11a"
DISSECTOR="3dpaxxel.lua"
PROTOCOL_NAME="3dp_axxel"
PROTOCOL_FILTER="xxx"

# [ "y" | "n" ] whether or not to use the user's dissector plugin directory.
# y: Lua dissector will be symlinked in the plugin directory
# n: Lua dissector will be passed as command line argument (existing symlink will be unlinked as well)
WIRESHARK_NO_USE_PLUGIN_DIR="y"
CLI_DISSECTOR_ARG="xxx"

# Sets the usb BUS_NR the device sits on and the device's DEV_NR on that bus.
function exportIds()
{
  local ids=$(lsusb -d ${VENDOR_ID}:${PRODUCT_ID} | \
        tr --delete ':' | \
        tr --squeeze '[:blank:]' ',' | \
        sed -E 's/^([[:alpha:]]+),([[:digit:]]+),([[:alpha:]]+),([[:digit:]]+).*$/BUS_NR=\2; DEV_NR=\4/')
  eval $ids
   # remove leading zeroes
  BUS_NR=$(echo $BUS_NR | bc)
  DEV_NR=$(echo $DEV_NR | bc)
}

# Creates plugin directory and updates dissector symlink or removes symlink
# according to WIRESHARK_NO_USE_PLUGIN_DIR.
function updateDissectorPluginDirectory()
{
  if [ "x$WIRESHARK_NO_USE_PLUGIN_DIR" == "xy" ] ; then
    echo "passing dissector by cli argument"
    unlink ${HOME}/.local/lib/wireshark/plugins/${DISSECTOR}
    CLI_DISSECTOR_ARG="-X lua_script:${SCRIPT_DIR}/${DISSECTOR}"
  else
    echo "using dissector plugins folder"
    mkdir --verbose --parents ${HOME}/.local/lib/wireshark/plugins
    ln --verbose --symbolic --force --no-target-directory \
      ${SCRIPT_DIR}/${DISSECTOR} \
      ${HOME}/.local/lib/wireshark/plugins/${DISSECTOR}
      CLI_DISSECTOR_ARG=""
  fi
}

function checkUsbmonLoaded()
{
  local module="usbmon"
  if [ -z $(lsmod | grep -Eo "^${module}") ] ; then
    echo "ERROR: module \"${module}\" not loaded (try \"sudo modprobe usbmon\" first)"
    exit -1
  fi
}

function lazyExpandFilter()
{
  PROTOCOL_FILTER="usb.device_address == ${DEV_NR} && ${PROTOCOL_NAME}"
}

CMD="$1"
case $CMD in
tshark)
  checkUsbmonLoaded
  exportIds
  lazyExpandFilter
  updateDissectorPluginDirectory

  set -x
  tshark --interface usbmon${BUS_NR} -Y "${PROTOCOL_FILTER}" --enable-protocol $PROTOCOL_NAME -O $PROTOCOL_NAME $CLI_DISSECTOR_ARG
  set +x

  ;;

wireshark)
  checkUsbmonLoaded
  exportIds
  lazyExpandFilter
  updateDissectorPluginDirectory

  set -x
  wireshark --interface usbmon${BUS_NR} -Y "${PROTOCOL_FILTER}"   --enable-protocol $PROTOCOL_NAME $CLI_DISSECTOR_ARG -k
  set +x
  ;;

nr)
  exportIds
  echo "BUS_NR=${BUS_NR}"
  echo "DEV_NR=${DEV_NR}"
  ;;

--help|*)
  echo "usage: $SCRIPT_NAME [ tshark | wireshark | nr ]"
  echo "       tshark:    starts tshark capture"
  echo "       wireshark: starts wireshark capture"
  echo "       id:        prints bus and device ID"

  if [ "$CMD" != "--help" ] ; then
    return -1
  fi
  ;;
esac
