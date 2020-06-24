package com.rndash.w203canbus

import java.io.IOException
import java.io.InputStream
import java.io.OutputStream
import java.util.UUID
import android.bluetooth.BluetoothAdapter
import android.bluetooth.BluetoothDevice
import android.bluetooth.BluetoothSocket
import android.util.Log
import java.lang.Exception
import java.nio.charset.Charset


// Bluetooth command ID's - Same as arduino

// Bluetooth command ID's from app
const val BT_CMD_TRACK_NAME : Byte = 0x00 // Track name provided
const val BT_CMD_MUSIC_CTRL : Byte = 0x01 // Control status provided
const val BT_CMD_TRACK_LEN : Byte =  0x02 // New track length provided
const val BT_CMD_TRACK_SEEK : Byte = 0x03 // New track seek position provided
const val BT_CMD_PING : Byte = 0xFF.toByte() // Ping to arduino

// Music control args - From phone app
const val  BT_PAUSE : Byte = 0x00
const val  BT_PLAY : Byte = 0x01

// Sent to phone on steering wheel command
const val  BT_NEXT : Byte = 0x02
const val  BT_PREV : Byte = 0x03

data class BTPayload(val cmd: Byte, val args: ByteArray);

class BluetoothComm(private var device: BluetoothDevice, private var secure: Boolean,
                    private var adapter: BluetoothAdapter) {

    var isConnected = false
    val readSerialBT = Thread() {
        while(true) {
            val x = readPayload()
            if (isConnected && x != null) {
                Log.i("BT", "Received payload: ${printPayload(x)}")
                when (x.cmd) {
                    BT_CMD_MUSIC_CTRL -> {
                        when(x.args[0]) {
                            BT_NEXT -> CarCommunicator.nextSong()
                            BT_PREV -> CarCommunicator.previousSong()
                        }
                    }

                }
            }
            try {
                Thread.sleep(10)
            } catch (e: InterruptedException) {
                break
            }
        }
    }
    private fun printPayload(a : BTPayload) : String {
        var x = "CMD: ["
        x += String.format("%02X", a.cmd)
        x += "] ARGS: ["
        a.args.forEach { x += String.format("%02X ", it.toInt()) }
        x += "]"
        return x
    }

    private lateinit var bluetoothSocket : BluetoothSocketWrapper
    private var uuid : UUID = UUID.fromString("00001101-0000-1000-8000-00805F9B34FB")

    private var payloadSize = 0;
    private fun readPayload() : BTPayload? {
        try {
            if (bluetoothSocket.getInputStream().available() > 0) {
                // New payload
                if (payloadSize == 0) {
                    payloadSize = bluetoothSocket.getInputStream().read();
                }
                if (bluetoothSocket.getInputStream().available() >= payloadSize) {
                    val buffer = ByteArray(payloadSize)
                    bluetoothSocket.getInputStream().read(buffer)
                    payloadSize = 0 // Reset counter
                    return BTPayload(buffer[0], buffer.drop(1).toByteArray())
                }
            }
        } catch (e: Exception) {
            e.printStackTrace()
        }
        return null
    }

    fun sendPayload(p: BTPayload) {
        try {
            Log.d("BT-SEND", "Sending Payload ${printPayload(p)}")
            with(bluetoothSocket.getOutputStream()) {
                write(1+p.args.size)
                write(byteArrayOf(p.cmd) + p.args)
                flush()
            }
        } catch (e: IOException) {
            Log.e("BT", "Arduino disconnected!")
            if (isConnected) {
                try {
                    bluetoothSocket.close()
                } catch (e: Exception) {
                    e.printStackTrace()
                }
            }
            isConnected = false
        }
    }

    fun disconnect() {
        isConnected = false
        readSerialBT.interrupt()
        bluetoothSocket.getInputStream().close()
        bluetoothSocket.getOutputStream().close()
        bluetoothSocket.close()
    }

    @Throws(IOException::class)
    fun connect() : BluetoothSocketWrapper {
        adapter.cancelDiscovery()
        while(selectSocket()) {
            try {
                bluetoothSocket.connect()
                isConnected = true
                break
            } catch (e: IOException) {
                try {
                    bluetoothSocket = FallbackBluetoothSocket(bluetoothSocket.getUnderlyingSocket())
                    Thread.sleep(500)
                    bluetoothSocket.connect()
                    isConnected = true
                    break
                } catch (e: FallbackException) {
                    Log.e("BT", "Cannot connect via fallback!")
                    break
                } catch (e: InterruptedException) {
                    Log.e("BT", "Interrupted!")
                    break
                } catch (e: IOException) {
                    Log.e("BT", "Fallback failed!. Cancelling", e)
                    break
                }
            }
        }
        if(!isConnected) {
            throw IOException("Cannot connect to device: ${device.address}")
        } else {
            readSerialBT.start()
        }
        return bluetoothSocket
    }

    private fun selectSocket() : Boolean {
        val uuid = this.uuid
        Log.i("BT", "Attempting to connect to Protocol: $uuid")
        val temp = if (secure) {
            device.createRfcommSocketToServiceRecord(uuid)
        } else {
            device.createInsecureRfcommSocketToServiceRecord(uuid)
        }
        bluetoothSocket = NativeBluetoothSocket(temp)
        return true
    }

    interface BluetoothSocketWrapper {
        fun getInputStream() : InputStream
        fun getOutputStream() : OutputStream
        fun getRemoteDeviceName() : String
        fun connect()
        fun getRemoteDeviceAddress() : String
        fun close()
        fun getUnderlyingSocket() : BluetoothSocket
    }

    open inner class NativeBluetoothSocket(private val socket: BluetoothSocket) : BluetoothSocketWrapper {
        init {
            Log.i("BT", "Creating Native bluetooth socket")
        }
        override fun getInputStream(): InputStream {
            return socket.inputStream
        }

        override fun getOutputStream(): OutputStream {
            return socket.outputStream
        }

        override fun close() {
            socket.close()
        }

        override fun connect() {
            socket.connect()
        }

        override fun getRemoteDeviceAddress(): String {
            return socket.remoteDevice.address
        }

        override fun getRemoteDeviceName(): String {
            return socket.remoteDevice.name
        }

        override fun getUnderlyingSocket(): BluetoothSocket {
            return socket
        }
    }

    inner class FallbackBluetoothSocket(tmp: BluetoothSocket) : BluetoothSocketWrapper {
        private val fallbackSocket: BluetoothSocket
        init {
            Log.i("BT", "Creating Fallback bluetooth socket")
            val clazz = tmp.remoteDevice.javaClass
            val types = arrayOf<Class<*>>(Integer.TYPE)
            val method = clazz.getMethod("createRfcommSocket", *types)
            fallbackSocket = method.invoke(tmp.remoteDevice, Integer.valueOf(1)) as BluetoothSocket
        }

        override fun close() {
            fallbackSocket.close()
        }

        override fun connect() {
            fallbackSocket.connect()
        }

        override fun getInputStream(): InputStream {
            return fallbackSocket.inputStream
        }

        override fun getOutputStream(): OutputStream {
            return fallbackSocket.outputStream
        }

        override fun getRemoteDeviceAddress(): String {
            return fallbackSocket.remoteDevice.address
        }

        override fun getRemoteDeviceName(): String {
            return fallbackSocket.remoteDevice.name
        }

        override fun getUnderlyingSocket(): BluetoothSocket {
            return fallbackSocket
        }
    }
    inner class FallbackException(private val e: Exception) : Exception() {
        override val cause: Throwable?
            get() = e.cause
        override val message: String?
            get() = e.message

        override fun fillInStackTrace(): Throwable {
            return e.fillInStackTrace()
        }

        override fun printStackTrace() {
            e.printStackTrace()
        }
    }
}