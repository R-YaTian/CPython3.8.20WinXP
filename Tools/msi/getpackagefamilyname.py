import ctypes
import os


def get_packagefamilyname(name, publisher_id):
    class PACKAGE_ID(ctypes.Structure):
        _fields_ = [
            ("reserved", ctypes.c_uint32),
            ("processorArchitecture", ctypes.c_uint32),
            ("version", ctypes.c_uint64),
            ("name", ctypes.c_wchar_p),
            ("publisher", ctypes.c_wchar_p),
            ("resourceId", ctypes.c_wchar_p),
            ("publisherId", ctypes.c_wchar_p),
        ]
        _pack_ = 4

    pid = PACKAGE_ID(0, 0, 0, name, publisher_id, None, None)
    result = ctypes.create_unicode_buffer(256)
    result_len = ctypes.c_uint32(256)
    r = ctypes.windll.kernel32.PackageFamilyNameFromId(
        pid, ctypes.byref(result_len), result
    )
    if r:
        raise OSError(r, "failed to get package family name")
    return result.value[: result_len.value]


if __name__ == "__main__":
    p_name = "PythonSoftwareFoundation.Python.3.8"
    p_publisher = os.getenv("APPX_DATA_PUBLISHER")
    pfn = get_packagefamilyname(p_name, p_publisher)
    print(pfn)
