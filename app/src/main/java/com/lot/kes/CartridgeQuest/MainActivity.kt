
package com.lot.kes.CartridgeQuest

import android.annotation.SuppressLint
import android.os.Build.VERSION
import android.os.Bundle
import android.os.Build
import android.view.View
import android.view.WindowManager.LayoutParams
import androidx.core.view.WindowInsetsCompat
import androidx.core.view.WindowInsetsControllerCompat

import com.google.androidgamesdk.GameActivity

class MainActivity : GameActivity() {
    override fun onResume() {
        super.onResume()
        hideSystemBars()
    }

    private fun hideSystemBars() {
        @SuppressLint("ObsoleteSdkInt")
        if (VERSION.SDK_INT >= Build.VERSION_CODES.P) {
            window.attributes.layoutInDisplayCutoutMode =
                LayoutParams.LAYOUT_IN_DISPLAY_CUTOUT_MODE_ALWAYS
        }
        val decorView: View = window.decorView
        val controller = WindowInsetsControllerCompat(
            window,
            decorView
        )
        controller.hide(WindowInsetsCompat.Type.systemBars())
        controller.hide(WindowInsetsCompat.Type.displayCutout())
        controller.systemBarsBehavior =
            WindowInsetsControllerCompat.BEHAVIOR_SHOW_TRANSIENT_BARS_BY_SWIPE
    }

    companion object {
        init {
            System.loadLibrary("game")
        }
    }
}