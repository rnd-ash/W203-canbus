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

class BluetoothComm(private var device: BluetoothDevice, private var secure: Boolean,
                    private var adapter: BluetoothAdapter) {

    private var isReading = false
    private var queue = ArrayList<String>()
    val readSerialBT = Thread() {
        Log.i("BT", "Reader thread started!")
        var buffer = ""
        while(true) {
            when (readString()) {
                "N" -> CarCommunicator.nextSong()
                "P" -> CarCommunicator.previousSong()
            }
            Thread.sleep(50)
        }
    }

    private lateinit var bluetoothSocket : BluetoothSocketWrapper
    private var uuid : UUID = UUID.fromString("00001101-0000-1000-8000-00805F9B34FB")

    private var buffer = ""
    private fun readString() : String {
        while (bluetoothSocket.getInputStream().available() > 0) {
            val char = bluetoothSocket.getInputStream().read().toChar()
            if (char == '\r') {
                Log.d("BT", "Read message $buffer")
                val tmp = buffer
                buffer = ""
                return tmp
            } else {
                buffer += char
            }
        }
        return buffer
    }

    fun sendString(msg: String) {
        val sendMsg = msg.toByteArray(Charsets.US_ASCII)
        val bytes = byteArrayOf(sendMsg.size.toByte()) + sendMsg
        Log.d("BT", "Sending message: '$msg'")
        bluetoothSocket.getOutputStream().write(bytes)
        bluetoothSocket.getOutputStream().flush()
    }

    fun disconnect() {
        readSerialBT.interrupt()
        bluetoothSocket.getInputStream().close()
        bluetoothSocket.getOutputStream().close()
        bluetoothSocket.close()
    }

    @Throws(IOException::class)
    fun connect() : BluetoothSocketWrapper {
        var success = false
        while(selectSocket()) {
            adapter.cancelDiscovery()
            try {
                bluetoothSocket.connect()
                success = true
                break
            } catch (e: IOException) {
                try {
                    bluetoothSocket = FallbackBluetoothSocket(bluetoothSocket.getUnderlyingSocket())
                    Thread.sleep(500)
                    bluetoothSocket.connect()
                    success = true
                    break
                } catch (e: FallbackException) {
                    Log.e("BT", "Cannot connect via fallback!")
                } catch (e: InterruptedException) {
                    Log.e("BT", "Interrupted!")
                } catch (e: IOException) {
                    Log.e("BT", "Fallback failed!. Cancelling", e)
                }
            }
        }
        if(!success) {
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