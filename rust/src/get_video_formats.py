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
