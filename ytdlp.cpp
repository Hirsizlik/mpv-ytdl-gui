#include "ytdlp.h"
#include <exception>
#include <iostream>
#define PY_SSIZE_T_CLEAN
#include <Python.h>

constexpr const char *const py_script =
    R"(
import yt_dlp
#import json

def get_video_formats(video_url: str,
                      password: str,
                      username: str,
                      cookiesBrowser: str) -> list[str]:
    yt_dlp_opts = {
        'username': username,
        'password': password,
        'cookiesfrombrowser': (cookiesBrowser, )
    }

    with yt_dlp.YoutubeDL(yt_dlp_opts) as ydl:
        info_dict = ydl.extract_info(url=video_url, download=False)
        # print(json.dumps(info_dict, indent=2))
        return (info_dict["duration"], info_dict["formats"])
)";

class DecrefOnExit {
  public:
    explicit DecrefOnExit(const PyObject *object): toDecref(object)
    {
    }
    ~DecrefOnExit()
    {
        Py_DECREF(toDecref);
    }

  private:
    const PyObject *toDecref;
};

yt::YtdlpData::YtdlpData(std::string &&id, std::string &&name, std::string &&resolution,
                         std::string &&vcodec, std::string &&acodec, unsigned long fps,
                         unsigned long long filesize, bool estimate, double tbr)
    : m_id(std::move(id)), m_name(std::move(name)), m_resolution(std::move(resolution)),
      m_vcodec(std::move(vcodec)), m_acodec(std::move(acodec)), m_fps(fps), m_filesize(filesize),
      m_filesize_estimate(estimate), m_tbr(tbr)
{
}

static constexpr double readDouble(PyObject *element) {
    if (PyFloat_Check(element)) {
        return PyFloat_AsDouble(element);
    } else if (PyLong_Check(element)) {
        return PyLong_AsDouble(element);
    } else {
        return 0;
    }
}

static yt::YtdlpData
parseData(PyObject *pyListCurrent, double duration)
{
    PyObject *pyElementId = PyDict_GetItemString(pyListCurrent, "format_id");
    PyObject *pyElementName = PyDict_GetItemString(pyListCurrent, "format");
    PyObject *pyElementResolution = PyDict_GetItemString(pyListCurrent, "resolution");
    PyObject *pyElementVcodec = PyDict_GetItemString(pyListCurrent, "vcodec");
    PyObject *pyElementAcodec = PyDict_GetItemString(pyListCurrent, "acodec");
    PyObject *pyElementFps = PyDict_GetItemString(pyListCurrent, "fps");
    PyObject *pyElementFilesize = PyDict_GetItemString(pyListCurrent, "filesize");
    PyObject *pyElementTbr = PyDict_GetItemString(pyListCurrent, "tbr");

    std::string vcodec;
    std::string acodec;
    std::string resolution;
    unsigned long fps = 0;
    unsigned long long filesize = 0;
    double tbr = 0;

    if (pyElementResolution && pyElementResolution != Py_None) {
        resolution = std::string(PyUnicode_AsUTF8(pyElementResolution));
    }
    if (pyElementVcodec && pyElementVcodec != Py_None) {
        vcodec = std::string(PyUnicode_AsUTF8(pyElementVcodec));
    }
    if (pyElementAcodec && pyElementAcodec != Py_None) {
        acodec = std::string(PyUnicode_AsUTF8(pyElementAcodec));
    }
    if (pyElementFps) {
        fps = readDouble(pyElementFps);
    }
    if (pyElementTbr) {
        tbr = readDouble(pyElementTbr);
    }

    bool filesizeEstimate = false;
    if (pyElementFilesize && (PyFloat_Check(pyElementFilesize) || PyLong_Check(pyElementFilesize))) {
        filesize = readDouble(pyElementFilesize);
    } else if (tbr > 0) {
        filesizeEstimate = true;
        filesize = readDouble(pyElementTbr) * 1024 / 8 * duration;
    }

    return yt::YtdlpData(std::string(PyUnicode_AsUTF8(pyElementId)),
                         std::string(PyUnicode_AsUTF8(pyElementName)),
                         std::move(resolution), std::move(vcodec),
                         std::move(acodec), fps, filesize, filesizeEstimate, tbr);
}

std::vector<yt::YtdlpData>
yt::loadFormats(const char *const username, const char *const password, const char *const url,
                const char *const cookiesBrowser)
{
    using namespace yt;
    Py_Initialize();
    PyObject *pyModule = PyImport_ImportModule("__main__");
    if (!pyModule)
        throw std::runtime_error("Error importing __main__");
    DecrefOnExit decModule(pyModule);

    PyObject *pyDict = PyModule_GetDict(pyModule);
    if (!pyDict)
        throw std::runtime_error("Error loading module dict");

    PyObject *pyFunc = PyDict_GetItemString(pyDict, "get_video_formats");
    if (!pyFunc) {
        if (PyRun_SimpleString(py_script))
            throw std::runtime_error("error occured durind script execution");

        pyFunc = PyDict_GetItemString(pyDict, "get_video_formats");
        if (!pyDict)
            throw std::runtime_error("Error loading module dict");
    }

    PyObject *pyArgs =
        PyTuple_Pack(4,
                     PyUnicode_FromString(url),
                     PyUnicode_FromString(password),
                     PyUnicode_FromString(username),
                     PyUnicode_FromString(cookiesBrowser));
    if (!pyArgs)
        throw std::runtime_error("Error creating tuple");

    PyObject *pyResult = PyObject_CallObject(pyFunc, pyArgs);
    DecrefOnExit decArgs(pyArgs);
    if (pyResult) {
        PyObject *pyDuration = PyTuple_GetItem(pyResult, 0);
        PyObject *pyFormats = PyTuple_GetItem(pyResult, 1);
        double duration = readDouble(pyDuration);
        const Py_ssize_t list_size = PyList_Size(pyFormats);
        std::vector<YtdlpData> result;
        for (Py_ssize_t i = 0; i < list_size; ++i) {
            PyObject *pyListCurrent = PyList_GetItem(pyFormats, i);
            result.emplace_back(parseData(pyListCurrent, duration));
        }
        return result;
    } else {
        if (PyErr_Occurred())
            PyErr_Print();
        throw std::runtime_error("Error while loading yt_dlp data");
    }
}
