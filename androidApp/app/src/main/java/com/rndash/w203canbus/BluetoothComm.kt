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

    var readText : String = ""
    private val btThread = Thread() {
        Log.i("BT", "Read thread started!")
        while(true) {
            var tmpTxt = ""
            while (bluetoothSocket.getInputStream().available() > 0) {
                tmpTxt += bluetoothSocket.getInputStream().read().toChar()
            }
            if (tmpTxt.isNotEmpty()) {
                readText = tmpTxt
                Log.d("BT", "Read message: $readText")
            }
            try {
                Thread.sleep(1000L)
            } catch (e: InterruptedException) {
                e.printStackTrace()
                break
            }
        }
    }

    private lateinit var bluetoothSocket : BluetoothSocketWrapper
    private var uuid : UUID = UUID.fromString("00001101-0000-1000-8000-00805F9B34FB")

    fun sendString(msg: String) {
        val id = (0..9).random()
        val sendMsg = "$id<$msg>"
        Log.i("BT", "Sending message '$msg'")
        bluetoothSocket.getOutputStream().write(sendMsg.toByteArray(Charsets.US_ASCII))
        bluetoothSocket.getOutputStream().flush()
        Log.i("BT", "Done sending message")
    }

    fun disconnect() {
        btThread.interrupt()
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
        }
        btThread.start()
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