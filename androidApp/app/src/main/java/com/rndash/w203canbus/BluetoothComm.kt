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

    var isConnected = false
    val readSerialBT = Thread() {
        Log.i("BT", "Reader thread started!")
        while(true) {
            val x = readString()
            if (isConnected && x.isNotEmpty()) {
                Log.i("BT", "Received bytes: ${printBytes(x)}")
                when (x[0].toInt()) {
                    0x00 -> CarCommunicator.nextSong()
                    0x01 -> CarCommunicator.previousSong()
                    0x03 -> {
                        Log.i("BT", "Requesting carrier")
                        sendBytes(byteArrayOf('C'.toByte()) + MainActivity.carrierName.toByteArray(Charsets.US_ASCII))
                    }
                }
            }
            try {
                Thread.sleep(50)
            } catch (e: InterruptedException) {
                break
            }
        }
    }
    private fun printBytes(a : ByteArray) : String {
        var x = ""
        a.forEach { x += String.format("%02X ", it.toInt()) }
        return x
    }

    private lateinit var bluetoothSocket : BluetoothSocketWrapper
    private var uuid : UUID = UUID.fromString("00001101-0000-1000-8000-00805F9B34FB")

    private fun readString() : ByteArray {
        var ret = ByteArray(0)
        try {
            while (bluetoothSocket.getInputStream().available() > 0) {
                ret += bluetoothSocket.getInputStream().read().toByte()
            }
        } catch (e: Exception) {
            e.printStackTrace()
        }
        return ret
    }

    fun sendString(msg: String) {
        val sendMsg = msg.toByteArray(Charsets.US_ASCII)
        sendMsg(sendMsg)
    }

    fun sendBytes(bytes: ByteArray) {
        Log.d("BT", "Sending Bytes: '${printBytes(bytes)}'")
        sendMsg(bytes);
    }

    private fun sendMsg(bytes: ByteArray) {
        val ba = byteArrayOf(bytes.size.toByte()) + bytes
        try {
            bluetoothSocket.getOutputStream().write(ba)
            bluetoothSocket.getOutputStream().flush()
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