use ffi::{VideoData, VideoFormat};
use pyo3::exceptions::PyTypeError;
use pyo3::prelude::*;
use pyo3::types::{PyDict, PyList, PyTuple};
use pyo3::DowncastIntoError;

const PY_SCRIPT: &str = include_str!("get_video_formats.py");

pub fn load_video_formats(
    username: &str,
    password: &str,
    url: &str,
    cookies_browser: &str,
    user_agent: &str,
) -> VideoData {
    pyo3::prepare_freethreaded_python();
    let result = Python::with_gil(|py| -> PyResult<VideoData> {
        let fun: Py<PyAny> = PyModule::from_code_bound(py, PY_SCRIPT, "", "")?
            .getattr("get_video_formats")?
            .into();
        let args = PyTuple::new_bound(py, &[url, username, password, cookies_browser, user_agent]);
        let fun_result = fun.call1(py, args)?;
        let vfs = fun_result.downcast_bound::<PyTuple>(py)?;
        let title: String = vfs.get_item(0)?.extract()?;
        let duration_item = vfs.get_item(1)?;
        let duration = duration_item.extract();
        let duration = match duration {
            Ok(d) => d,
            Err(_) => duration_item.extract::<f64>()? as u64,
        };

        let format_list: Bound<PyList> = vfs.get_item(2)?.extract()?;

        let r = format_list
            .iter()
            .map(|i| i.downcast_into::<PyDict>())
            .collect::<Result<Vec<Bound<PyDict>>, DowncastIntoError>>()?
            .iter()
            .map(|i| read_list_item(i, duration))
            .collect::<Result<Vec<VideoFormat>, PyErr>>()?;
        Ok(VideoData {
            name: title,
            duration,
            formats: r,
        })
    });

    match result {
        Err(e) => {
            println!("{e}");
            VideoData {
                name: "".to_string(),
                duration: 0,
                formats: vec![],
            }
        }
        Ok(r) => r,
    }
}

fn read_list_item(list_item: &Bound<PyDict>, duration: u64) -> PyResult<VideoFormat> {
    let id: String = list_item
        .get_item("format_id")?
        .ok_or(PyTypeError::new_err("No format_id"))?
        .extract()?;
    let name: String = list_item
        .get_item("format")?
        .ok_or(PyTypeError::new_err("no format name"))?
        .extract()?;
    let resolution: String = get_or_default(list_item, "resolution", "".to_string())?;
    let vcodec: String = get_or_default(list_item, "vcodec", "".to_string())?;
    let acodec: String = get_or_default(list_item, "acodec", "".to_string())?;
    let fps: u32 = get_or_default(list_item, "fps", 0.0)? as u32;
    let tbr: f64 = get_or_default(list_item, "tbr", 0.0)?;
    let filesize_is_estimate: bool;
    let filesize: u64;

    match list_item.get_item("filesize")?.filter(|n| !n.is_none()) {
        None => {
            filesize = (duration as f64 * tbr * 1024.0 / 8.0) as u64;
            filesize_is_estimate = true;
        }
        Some(v) => {
            filesize = v.extract()?;
            filesize_is_estimate = false;
        }
    }

    Ok(VideoFormat {
        id,
        name,
        resolution,
        vcodec,
        acodec,
        fps,
        filesize,
        filesize_is_estimate,
        tbr,
    })
}

fn get_or_default<'a, T>(list_item: &'a Bound<PyDict>, key: &str, default: T) -> Result<T, PyErr>
where
    T: pyo3::FromPyObject<'a>,
{
    let r: T = match list_item.get_item(key)?.filter(|n| !n.is_none()) {
        None => default,
        Some(v) => v.extract()?,
    };
    Ok(r)
}

#[cxx::bridge]
mod ffi {

    struct VideoData {
        name: String,
        duration: u64,
        formats: Vec<VideoFormat>,
    }
    struct VideoFormat {
        id: String,
        name: String,
        resolution: String,
        vcodec: String,
        acodec: String,
        fps: u32,
        filesize: u64,
        filesize_is_estimate: bool,
        tbr: f64,
    }

    extern "Rust" {
        fn load_video_formats(
            username: &str,
            password: &str,
            url: &str,
            cookies_browser: &str,
            user_agent: &str,
        ) -> VideoData;
    }
}
