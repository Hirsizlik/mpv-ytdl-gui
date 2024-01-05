#ifndef MPV_YTDL_GUI_YTDLP_H
#define MPV_YTDL_GUI_YTDLP_H

#include <string>
#include <vector>

namespace yt {

class YtdlpData {
  public:
    explicit YtdlpData(std::string &&id, std::string &&name, std::string &&resolution,
                       std::string &&vcodec, std::string &&acodec, unsigned long fps,
                       unsigned long long filesize, bool estimate, double tbr);
    constexpr std::string id() const
    {
        return m_id;
    }
    constexpr std::string name() const
    {
        return m_name;
    }
    constexpr std::string resolution() const
    {
        return m_resolution;
    }
    constexpr std::string vcodec() const
    {
      return m_vcodec;
    }
    constexpr std::string acodec() const
    {
      return m_acodec;
    }
    constexpr unsigned long fps() const
    {
      return m_fps;
    }
    constexpr unsigned long long filesize() const
    {
      return m_filesize;
    }
    constexpr bool filesize_estimate() const
    {
      return m_filesize_estimate;
    }
    constexpr double tbr() const
    {
      return m_tbr;
    }

  private:
    const std::string m_id;
    const std::string m_name;
    const std::string m_resolution;
    const std::string m_vcodec;
    const std::string m_acodec;
    const unsigned long m_fps;
    const unsigned long long m_filesize;
    const bool m_filesize_estimate;
    const double m_tbr;
};

// all arguments should be UTF-8 encoded, with a trailing '\0'
//
// returns a vector of YtdlpData, strings within are UTF-8 encoded
std::vector<YtdlpData> loadFormats(const char *const username, const char *const password,
                                   const char *const url, const char *const cookiesBrowser);
} // namespace yt

#endif
