package by.diplom.vibric_mobile_v2.utils

import android.os.Build
import androidx.annotation.RequiresApi
import java.text.SimpleDateFormat
import java.time.format.DateTimeFormatter
import java.util.Date
import java.util.Locale

fun getCurrentUtc(): String {
    val pattern = "yyMMddHHmmss"
    val simpleDateFormat = SimpleDateFormat(pattern, Locale.getDefault())
    return simpleDateFormat.format(Date())
}