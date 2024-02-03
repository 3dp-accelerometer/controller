-- References:
--   - introduction into Lua dissector: https://mika-s.github.io/wireshark/lua/dissector/2017/11/04/creating-a-wireshark-dissector-in-lua-1.html
--   - register dissector for USB: https://egeek.me/2014/03/01/wireshark-setting-up-dissector-for-usb-aoa-packets/
--   - usb.transfer_type, usb.endpoint_address.direction: https://wiki.wireshark.org/USB

axxelProtocol = Proto("3DP_Axxel", "3D Printer Accelerometer Protocol")

-- device Id: VendorId + ProductId
local devIdVendor   = 0x1209
local devIdModel    = 0xe11a
local devId         = devIdVendor * 2^16 + devIdModel

-- filtering: URB_BULK transfer type and IN direction
local fUsbTransferType       = Field.new("usb.transfer_type")
local fUsbEndpointDirection  = Field.new("usb.endpoint_address.direction")
local usbTransferTypeBulk    = 0x03
local usbEndpointDirectionIn = 0x01

-- header name to ID mapping for each known 3DP Accelerometer package
local headerNameToId = {
    -- configuration (tx)
    ["TX_SET_OUTPUT_DATA_RATE"]     =  1,
    ["TX_GET_OUTPUT_DATA_RATE"]     =  2,
    ["TX_SET_RANGE"]                =  3,
    ["TX_GET_RANGE"]                =  4,
    ["TX_SET_SCALE"]                =  5,
    ["TX_GET_SCALE"]                =  6,
    ["TX_GET_DEVICE_SETUP"]         =  7,
    ["TX_GET_FIRMWARE_VERSION"]     =  8,
    ["TX_GET_UPTIME"]               =  9,
    ["TX_GET_BUFFER_STATUS"]        = 10,
    -- sampling (tx)
    ["TX_DEVICE_REBOOT"]            = 17,
    ["TX_SAMPLING_START"]           = 18,
    ["TX_SAMPLING_STOP"]            = 19,
    -- configuration (rx)
    ["RX_OUTPUT_DATA_RATE"]         = 25,
    ["RX_RANGE"]                    = 26,
    ["RX_SCALE"]                    = 27,
    ["RX_DEVICE_SETUP"]             = 28,
    ["RX_FIRMWARE_VERSION"]         = 29,
    ["RX_UPTIME"]                   = 30,
    ["RX_BUFFER_STATUS"]            = 31,
    -- sampling (rx)
    ["RX_SAMPLING_FIFO_OVERFLOW"]   = 33,
    ["RX_SAMPLING_STARTED"]         = 34,
    ["RX_SAMPLING_FINISHED"]        = 35,
    ["RX_SAMPLING_STOPPED"]         = 36,
    ["RX_SAMPLING_ABORTED"]         = 37,
    ["RX_ACCELERATION"]             = 38,
    ["RX_FAULT"]                    = 39,
    ["RX_SAMPLING_BUFFER_OVERFLOW"] = 40,
    ["RX_TRANSMISSION_ERROR"]       = 41,
}

-- header ID to name mapping for each known 3DP Accelerometer package
local headerIdToName = {
    -- configuration (tx)
    [headerNameToId.TX_SET_OUTPUT_DATA_RATE]     = "TX_SET_OUTPUT_DATA_RATE",
    [headerNameToId.TX_GET_OUTPUT_DATA_RATE]     = "TX_GET_OUTPUT_DATA_RATE",
    [headerNameToId.TX_SET_RANGE]                = "TX_SET_RANGE",
    [headerNameToId.TX_GET_RANGE]                = "TX_GET_RANGE",
    [headerNameToId.TX_SET_SCALE]                = "TX_SET_SCALE",
    [headerNameToId.TX_GET_SCALE]                = "TX_GET_SCALE",
    [headerNameToId.TX_GET_DEVICE_SETUP]         = "TX_GET_DEVICE_SETUP",
    [headerNameToId.TX_GET_FIRMWARE_VERSION]     = "TX_GET_FIRMWARE_VERSION",
    [headerNameToId.TX_GET_UPTIME]               = "TX_GET_UPTIME",
    [headerNameToId.TX_GET_BUFFER_STATUS]        = "TX_GET_BUFFER_STATUS",
    -- sampling (tx)
    [headerNameToId.TX_DEVICE_REBOOT]            = "TX_DEVICE_REBOOT",
    [headerNameToId.TX_SAMPLING_START]           = "TX_SAMPLING_START",
    [headerNameToId.TX_SAMPLING_STOP]            = "TX_SAMPLING_STOP",
    -- configuration (rx)
    [headerNameToId.RX_OUTPUT_DATA_RATE]         = "RX_OUTPUT_DATA_RATE",
    [headerNameToId.RX_RANGE]                    = "RX_RANGE",
    [headerNameToId.RX_SCALE]                    = "RX_SCALE",
    [headerNameToId.RX_DEVICE_SETUP]             = "RX_DEVICE_SETUP",
    [headerNameToId.RX_FIRMWARE_VERSION]         = "RX_FIRMWARE_VERSION",
    [headerNameToId.RX_UPTIME]                   = "RX_UPTIME",
    [headerNameToId.RX_BUFFER_STATUS]            = "RX_BUFFER_STATUS",
    -- sampling (rx)
    [headerNameToId.RX_SAMPLING_FIFO_OVERFLOW]   = "RX_SAMPLING_FIFO_OVERFLOW",
    [headerNameToId.RX_SAMPLING_STARTED]         = "RX_SAMPLING_STARTED",
    [headerNameToId.RX_SAMPLING_FINISHED]        = "RX_SAMPLING_FINISHED",
    [headerNameToId.RX_SAMPLING_STOPPED]         = "RX_SAMPLING_STOPPED",
    [headerNameToId.RX_SAMPLING_ABORTED]         = "RX_SAMPLING_ABORTED",
    [headerNameToId.RX_ACCELERATION]             = "RX_ACCELERATION",
    [headerNameToId.RX_FAULT]                    = "RX_FAULT",
    [headerNameToId.RX_SAMPLING_BUFFER_OVERFLOW] = "RX_SAMPLING_BUFFER_OVERFLOW",
    [headerNameToId.RX_TRANSMISSION_ERROR]       = "RX_TRANSMISSION_ERROR",
}

-- sensor ODR field names
local sensorOutputDataRateFlagToName = {
    [15] = "ODR_3200",
    [14] = "ODR_1600",
    [13] = "ODR_800",
    [12] = "ODR_400",
    [11] = "ODR_200",
    [10] = "ODR_100",
    [ 9] = "ODR_50"
}

-- sensor range field names
local sensorRangeFlagToName = {
    [0] =  "2g",
    [1] =  "4g",
    [2] =  "8g",
    [3] = "16g"
}

-- sensor scale field names
local sensorScaleToName = {
    [0] = "10BIT",
    [1] = "FULL_RES_4MG_LSB"
}

-- device fault codes
local faultCodeToName = {
    [0] = "NmiHandler",
    [1] = "UsageFaultHandler",
    [2] = "BusFaultHandler",
    [3] = "HardFaultHandler",
    [4] = "ErrorHandler"
}

-- TX/RX header fields
pfHeaderId = ProtoField.uint8("axxel.headerId", "headerId", base.HEX, headerIdToName)
-- RX buffer status fields
pfBufferStatusSizeBytes           = ProtoField.uint16("axxel.bufferStatus.sizeBytes",           "sizeBytes",           base.DEC)
pfBufferStatusCapacityTotal       = ProtoField.uint16("axxel.bufferStatus.capacityTotal",       "capacityTotal",       base.DEC)
pfBufferStatusCapacityUsedMax     = ProtoField.uint16("axxel.bufferStatus.capacityUsedMax",     "capacityUsedMax",     base.DEC)
pfBufferStatusPutCount            = ProtoField.uint16("axxel.bufferStatus.putCount",            "putCount",            base.DEC)
pfBufferStatusTakeCount           = ProtoField.uint16("axxel.bufferStatus.takeCount",           "takeCount",           base.DEC)
pfBufferStatusLargestTxChunkBytes = ProtoField.uint16("axxel.bufferStatus.largestTxChunkBytes", "largestTxChunkBytes", base.DEC)
-- RX firmware version fields
pfFirmwareVersionMajor = ProtoField.uint8("axxel.firmwareVersion.major", "major", base.DEC)
pfFirmwareVersionMinor = ProtoField.uint8("axxel.firmwareVersion.minor", "minor", base.DEC)
pfFirmwareVersionPatch = ProtoField.uint8("axxel.firmwareVersion.patch", "patch", base.DEC)
-- RX sensor
pfSensorOutputDataRate = ProtoField.uint8("axxel.sensor.odr", "rate", base.HEX, sensorOutputDataRateFlagToName, 0x0f)
pfSensorRange = ProtoField.uint8("axxel.sensor.range", "range",       base.HEX, sensorRangeFlagToName, 0x03)
pfSensorScale = ProtoField.uint8("axxel.sensor.scale", "scale",       base.HEX, sensorScaleToName,              0x01)
-- RX device setup
pfDeviceSetupSensorOutputDataRate = ProtoField.uint8("axxel.deviceSetup.outputDataRate", "outputDataRate", base.HEX, sensorOutputDataRateFlagToName, 0x0f)
pfDeviceSetupSensorRange = ProtoField.uint8("axxel.deviceSetup.range", "range",                            base.HEX, sensorRangeFlagToName, 0x30)
pfDeviceSetupSensorScale = ProtoField.uint8("axxel.deviceSetup.scale", "scale",                            base.HEX, sensorScaleToName,              0x40)
-- RX device uptime
pfDeviceUptime = ProtoField.uint32("axxel.deviceUptime.elapsedMs", "elapsedMs", base.DEC)
-- RX device fault codes
pfDeviceFault = ProtoField.uint8("axxel.fault.code", "code", base.HEX, faultCodeToName)
-- RX acceleration
pfAccelerationX = ProtoField.uint16("axxel.acceleration.x", "x", base.DEC)
pfAccelerationY = ProtoField.uint16("axxel.acceleration.y", "y", base.DEC)
pfAccelerationZ = ProtoField.uint16("axxel.acceleration.z", "z", base.DEC)

-- protocol fields
axxelProtocol.fields = {
    -- header
    pfHeaderId,
    -- buffer
    pfBufferStatusSizeBytes,
    pfBufferStatusCapacityTotal,
    pfBufferStatusCapacityUsedMax,
    pfBufferStatusPutCount,
    pfBufferStatusTakeCount,
    pfBufferStatusLargestTxChunkBytes,
    pfFirmwareVersionMajor,
    pfFirmwareVersionMinor,
    pfFirmwareVersionPatch,
    pfSensorOutputDataRate,
    pfSensorRange,
    pfSensorScale,
    pfDeviceSetupSensorOutputDataRate,
    pfDeviceSetupSensorRange,
    pfDeviceSetupSensorScale,
    pfDeviceUptime,
    pfDeviceFault,
    pfAccelerationX,
    pfAccelerationY,
    pfAccelerationZ

}

-- expert fields
local efRequest     = ProtoExpert.new("axxel.direction.request",  "host request",        expert.group.REQUEST_CODE,  expert.severity.CHAT)
local efResponse    = ProtoExpert.new("axxel.direction.response", "controller response", expert.group.RESPONSE_CODE, expert.severity.CHAT)
local efBadRequest  = ProtoExpert.new("axxel.request.bad",        "unknown request",     expert.group.MALFORMED,     expert.severity.ERROR)
local efBadResponse = ProtoExpert.new("axxel.response.bad",       "unknown response",    expert.group.MALFORMED,     expert.severity.ERROR)

axxelProtocol.experts = { efRequest, efResponse, efBadRequest, efBadResponse }

-- decode the buffer status payload
function decodeBufferStatus(buffer, tree)
    local payloadTree = tree:add(axxelProtocol, buffer(), "Buffer Status")
    payloadTree:add_le(pfBufferStatusSizeBytes,           buffer( 0,2))
    payloadTree:add_le(pfBufferStatusCapacityTotal,       buffer( 2,2))
    payloadTree:add_le(pfBufferStatusCapacityUsedMax,     buffer( 4,2))
    payloadTree:add_le(pfBufferStatusPutCount,            buffer( 6,2))
    payloadTree:add_le(pfBufferStatusTakeCount,           buffer( 8,2))
    payloadTree:add_le(pfBufferStatusLargestTxChunkBytes, buffer(10,2))
end

-- decode the firmware version payload
function decodeFirmwareVersion(buffer, tree)
    local payloadTree = tree:add(axxelProtocol, buffer(), "Firmware Version")
    payloadTree:add_le(pfFirmwareVersionMajor, buffer(0,1))
    payloadTree:add_le(pfFirmwareVersionMinor, buffer(1,1))
    payloadTree:add_le(pfFirmwareVersionPatch, buffer(2,1))
end

-- decode the device setup payload
function decodeDeviceSetup(buffer, tree)
    local payloadTree = tree:add(axxelProtocol, buffer(), "Device Setup")
    payloadTree:add_le(pfDeviceSetupSensorOutputDataRate, buffer(0,1))
    payloadTree:add_le(pfDeviceSetupSensorRange,          buffer(0,1))
    payloadTree:add_le(pfDeviceSetupSensorScale,          buffer(0,1))
end

-- decode the sensor output data rate payload
function decodeSensorOutputDataRate(buffer, tree)
    local payloadTree = tree:add(axxelProtocol, buffer(), "Sensor Output Data Rate")
    payloadTree:add_le(pfSensorOutputDataRate, buffer(0,1))
end

-- decode the sensor range payload
function decodeSensorRange(buffer, tree)
    local payloadTree = tree:add(axxelProtocol, buffer(), "Sensor Range")
    payloadTree:add_le(pfSensorRange, buffer(0,1))
end

-- decode the sensor scale payload
function decodeSensorScale(buffer, tree)
    local payloadTree = tree:add(axxelProtocol, buffer(), "Sensor Scale")
    payloadTree:add_le(pfSensorScale, buffer(0,1))
end

-- decode the device uptime payload
function decodeDeviceUptime(buffer, tree)
    local payloadTree = tree:add(axxelProtocol, buffer(), "Device Uptime")
    payloadTree:add_le(pfDeviceUptime, buffer(0,1))
end

-- decode the device fault code payload
function decodeDeviceFaultCode(buffer, tree)
    local payloadTree = tree:add(axxelProtocol, buffer(), "Device Fault")
    payloadTree:add_le(pfDeviceFault, buffer(0,1))
end

-- decode the acceleration payload
function decodeAcceleration(buffer, tree)
    local payloadTree = tree:add(axxelProtocol, buffer(), "Acceleration")
    payloadTree:add_le(pfAccelerationX, buffer(0,2))
    payloadTree:add_le(pfAccelerationY, buffer(2,2))
    payloadTree:add_le(pfAccelerationZ, buffer(4,2))
end

-- decode fields and sub-fields
function axxelProtocol.dissector(buffer, pinfo, tree)
    length = buffer:len()
    if 0 == length then return end

    local usbTransferType = tonumber(tostring(fUsbTransferType()))
    if usbTransferTypeBulk ~= usbTransferType then return end

    local usbEndpointDirection = tonumber(tostring(fUsbEndpointDirection()))

    -- requests from host to controller (direction: out)
    if usbEndpointDirectionIn ~= usbEndpointDirection then
        pinfo.cols.protocol = axxelProtocol.name

        local dataTree = tree:add(axxelProtocol, buffer(), "3DP Axxel Data (request)")
        local id = buffer(0,1):uint()
        dataTree:add_le(pfHeaderId, id)
        dataTree:add_proto_expert_info(efRequest)

        if nil == headerIdToName[id] then
            dataTree:add_proto_expert_info(efBadRequest, "unknown request headerId (" .. string.format("0x%x", id) .. ")")
        end

    -- responses from controller (direction: in)
    elseif usbEndpointDirectionIn ~= nil then
        pinfo.cols.protocol = axxelProtocol.name

        local dataTree = tree:add(axxelProtocol, buffer(), "3DP Axxel Data (response)")
        local id = buffer(0,1):uint()
        dataTree:add_le(pfHeaderId, id)

        dataTree:add_proto_expert_info(efResponse)
        if id == headerNameToId.RX_BUFFER_STATUS then
            decodeBufferStatus(buffer(1), dataTree)
        elseif id == headerNameToId.RX_FIRMWARE_VERSION then
            decodeFirmwareVersion(buffer(1), dataTree)
        elseif id == headerNameToId.RX_DEVICE_SETUP then
            decodeDeviceSetup(buffer(1), dataTree)
        elseif id == headerNameToId.RX_OUTPUT_DATA_RATE then
            decodeSensorOutputDataRate(buffer(1), dataTree)
        elseif id == headerNameToId.RX_RANGE then
            decodeSensorRange(buffer(1), dataTree)
        elseif id == headerNameToId.RX_SCALE then
            decodeSensorScale(buffer(1), dataTree)
        elseif id == headerNameToId.RX_UPTIME then
            decodeDeviceUptime(buffer(1), dataTree)
        else
            dataTree:add_proto_expert_info(efBadResponse, "unknown response headerId (" .. string.format("0x%x", id) .. ")")
        end
    else
        print("xxx warning")
    end
end

-- register dissector
local success, data = pcall(function() return DissectorTable.get("usb.product") end)
if success then
  print("registering protocol for usbId=" .. devId)
  local usbProductDissectorTable = data
  usbProductDissectorTable:add(devId, axxelProtocol)
else
  print("registering protocol for USB device class")
  -- register the dissector for USB device class

  -- from packet-usb.h
  local IF_CLASS_DEVICE          = 0x0000
  local IF_CLASS_VENDOR_SPECIFIC = 0x00ff
  local IF_CLASS_UNKNOWN         = 0xffff

  local usbBulkDissectorTable = DissectorTable.get("usb.bulk")
  usbBulkDissectorTable:add(IF_CLASS_DEVICE,          axxelProtocol)
  usbBulkDissectorTable:add(IF_CLASS_VENDOR_SPECIFIC, axxelProtocol)
  usbBulkDissectorTable:add(IF_CLASS_UNKNOWN,         axxelProtocol)
end
