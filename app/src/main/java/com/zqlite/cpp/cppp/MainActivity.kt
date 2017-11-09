package com.zqlite.cpp.cppp

import android.graphics.Bitmap
import android.support.v7.app.AppCompatActivity
import android.os.Bundle
import android.util.Log
import kotlinx.android.synthetic.main.activity_main.*
import java.io.ByteArrayOutputStream
import java.util.*

class MainActivity : AppCompatActivity() {

    var currentSeek: Long = -1
    var seekSize: Long = 0
    var gifParserAddress: Long = 0
    var mBitmapArray: IntArray? = null
    var W = 0
    var H = 0
    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        setContentView(R.layout.activity_main)


        Thread(Runnable {
            val inputStram = assets.open("c.GIF")
            val bos = ByteArrayOutputStream()
            val buffer = ByteArray(1024)
            var len: Int
            do {
                len = inputStram.read(buffer)
                if (len > 0) {
                    bos.write(buffer, 0, len)
                }
            } while (len > 0)
            inputStram.close()
            initGif(bos.toByteArray(), bos.size())
            bos.close()
        }).start()
        // Example of a call to a native method
        btn_n.setOnClickListener {
            if (seekSize != 0L) {
                currentSeek += 1
                if (currentSeek == seekSize) {
                    currentSeek = 0
                }
                seekIddTo(currentSeek, gifParserAddress)
            }

        }
        btn_p.setOnClickListener {
            if (seekSize != 0L) {
                currentSeek -= 1
                if (currentSeek < 0) {
                    currentSeek = 0
                }
                seekIddTo(currentSeek, gifParserAddress)
            }
        }
    }

    fun gifLoadFinish(size: Long, w: Int, h: Int) {
        Log.d("gif", "gif idd size : $size  screen w = $w    h =  $h")
        mBitmapArray = kotlin.IntArray(w * h)
        seekSize = size
        W = w
        H = h
    }

    fun saveGifParserAddress(address: Long) {
        gifParserAddress = address
        Log.d("gif", "gif parser address : " + gifParserAddress)
    }

    fun drawOneFrame(top: Int, left: Int, w: Int, h: Int, bitmapArray: IntArray) {
        Log.d("gif", "top = " + top)
        Log.d("gif", "left = " + left)
        Log.d("gif", "w = " + w)
        Log.d("gif", "h = " + h)
        bitmapArray.forEach {
            //Log.d("gif", "-color = " + it)
        }
        var tmpIndex = 0
        for (i in top until top + h) {
            for (j in left until left+w) {
                val index = W * i + j
                if(tmpIndex < bitmapArray.size){
                    val color = bitmapArray[tmpIndex]

                    //Log.d("gif","top = $top , left = $left , w = $w , h = $h ,   i = $i , j = $j , tmpIndex = $tmpIndex  ，color = ${Integer.toBinaryString(color)}")
                    mBitmapArray!![index] = color
                }

                tmpIndex++
            }

        }
        val bitmap: Bitmap = Bitmap.createBitmap(mBitmapArray, W, H, Bitmap.Config.RGB_565)
        imageView.setImageBitmap(bitmap)
    }

    /**
     * A native method that is implemented by the 'native-lib' native library,
     * which is packaged with this application.
     */
    external fun stringFromJNI(): String

    external fun initGif(gifBytes: ByteArray, size: Int)
    external fun seekIddTo(position: Long, gifAddress: Long)

    companion object {

        // Used to load the 'native-lib' library on application startup.
        init {
            System.loadLibrary("native-lib")
        }
    }
}
