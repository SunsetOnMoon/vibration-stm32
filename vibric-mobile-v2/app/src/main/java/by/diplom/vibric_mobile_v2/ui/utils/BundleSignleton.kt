package by.diplom.vibric_mobile_v2.ui.utils

import android.os.Bundle

object BundleSingleton {
    private val bundle = Bundle()

    fun getBundle(): Bundle = bundle
    fun putString(key: String, value: String) {
        bundle.putString(key, value)
    }
    fun getString(key: String): String? {
        return bundle.getString(key)
    }

    fun putInt(key: String, value: Int) {
        bundle.putInt(key, value)
    }
    fun getInt(key: String): Int {
        return bundle.getInt(key)
    }

    fun putBoolean(key: String, value: Boolean) {
        bundle.putBoolean(key, value)
    }
    fun getBoolean(key: String): Boolean {
        return bundle.getBoolean(key)
    }
}