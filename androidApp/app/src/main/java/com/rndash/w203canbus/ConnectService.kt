package com.rndash.w203canbus

import android.app.*
import android.bluetooth.BluetoothAdapter
import android.content.Context
import android.content.Intent
import android.graphics.Color
import android.os.Build
import android.os.IBinder
import android.util.Log
import android.widget.Toast
import androidx.annotation.RequiresApi

class ConnectService() : Service() {
    val id = 543
    companion object {
        lateinit var ic: CarCommunicator
        var isCreate = false
    }
    override fun onCreate() {
        super.onCreate()
    }

    override fun onStart(intent: Intent?, startId: Int) {
        intent?.let {
            when(it.action) {
                "disconnect" -> {
                    disconnectedNotification()
                }
                "scan" -> {
                    scanningNotification()
                }
                "connect" -> {
                    ConnectNotification()
                }
            }
        }
    }

    override fun onBind(intent: Intent?): IBinder? {
        return null
    }

    fun scanningNotification() {
        val channelId =
            if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.O) {
                createNotificationChannel("my_service", "My Background Service")
            } else {
                ""
            }
        val notification = Notification.Builder(this, channelId)
            .setContentIntent(PendingIntent.getActivity(this, 0, Intent(this, MainActivity::class.java), PendingIntent.FLAG_UPDATE_CURRENT))
            .setColor(Color.YELLOW)
            .setContentTitle("W203 Canbus")
            .setContentText("Service running. Scanning for Arduino")
            .setSmallIcon(R.drawable.ic_launcher_foreground)
            .build()
        startForeground(id, notification)
    }

    fun ConnectNotification() {
        val channelId =
            if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.O) {
                createNotificationChannel("service", "Arduino notification service")
            } else {
                ""
            }
        val notification = Notification.Builder(this, channelId)
            .setContentIntent(PendingIntent.getActivity(this, 0, Intent(this, MainActivity::class.java), PendingIntent.FLAG_UPDATE_CURRENT))
            .setColor(Color.GREEN)
            .setContentTitle("W203 Canbus")
            .setContentText("Serivce running. Connected to Arduino")
            .setSmallIcon(R.drawable.ic_launcher_foreground)
            .build()
        startForeground(id, notification)
    }

    fun disconnectedNotification() {
        val channelId =
            if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.O) {
                createNotificationChannel("my_service", "My Background Service")
            } else {
                ""
            }
        val notification = Notification.Builder(this, channelId)
            .setColor(Color.RED)
            .setContentIntent(PendingIntent.getActivity(this, 0, Intent(this, MainActivity::class.java), PendingIntent.FLAG_UPDATE_CURRENT))
            .setContentTitle("W203 Canbus")
            .setContentText("Service running. Not connected to Arduino")
            .setSmallIcon(R.drawable.ic_launcher_foreground)
            .build()
        startForeground(id, notification)
    }
    @RequiresApi(Build.VERSION_CODES.O)
    private fun createNotificationChannel(channelId: String, channelName: String): String{
        val chan = NotificationChannel(channelId, channelName, NotificationManager.IMPORTANCE_LOW)
        chan.lightColor = Color.BLUE
        chan.lockscreenVisibility = Notification.VISIBILITY_PRIVATE
        val service = getSystemService(Context.NOTIFICATION_SERVICE) as NotificationManager
        service.createNotificationChannel(chan)
        return channelId
    }

    override fun onDestroy() {
        super.onDestroy()
        try {
            if (ic.btManager.isConnected) {
                ic.btManager.disconnect()
            }
        } catch (e: UninitializedPropertyAccessException) {
        }
        val n = applicationContext.getSystemService(Context.NOTIFICATION_SERVICE) as NotificationManager
        n.cancel(id)
    }
}