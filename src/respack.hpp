#ifndef HUES_RESPACK_H_
#define HUES_RESPACK_H_

#include <cmath>
#include <locale>
#include <string>
#include <vector>

#include <png.h>

#include <common.hpp>

using namespace std;

class ImageResource;
class AudioResource;

class ResourcePack {
  DISALLOW_COPY_AND_ASSIGN(ResourcePack)

public:

  /**
   * Create a new ResourcePack whose base directory is path.
   *
   * @param path the root path of the resource pack.
   */
  ResourcePack(const string& path) : base_path(path) { }
  ~ResourcePack();

  /**
   * Initialize the ResourcePack, parsing all XML metadata and making the List* methods callable.
   *
   * @return <code>true</code> if initialization was successful, <code>false</code> otherwise.
   */
  bool Init();

  /**
   * Populate a vector with a list of loops (including buildups) present in this resource pack.
   *
   * Note that buildups are NOT counted as a separate song, but instead are returned with (and
   * loaded with) the main loop.
   *
   * @param song_list a vector to place the song list into.
   * @return the number of songs present in this resource pack.
   */
  int GetAllSongs(vector<AudioResource*>& song_list) const;
  /**
   * Populate a vector with a list of images present in this resource pack.
   *
   * @param images_list a vector to place the images list into.
   * @return the number of images present in this resource pack.
   */
  int GetAllImages(vector<ImageResource*>& image_list) const;

  string GetBasePath() const;

private:
  void ParseSongXmlFile();
  void ParseImageXmlFile();

  const string base_path;

  vector<AudioResource*> song_list;
  vector<ImageResource*> image_list;
};

class ImageResource {
  DISALLOW_COPY_AND_ASSIGN(ImageResource)

  friend class ResourcePack;

public:

  /** Alignment for upscaled images and non-matching aspect ratios. */
  enum class Align {
    LEFT,
    CENTER,
    RIGHT
  };

  /**
   * Constructs a new, named ImageResource.
   *
   * @param base_path the base path of the containing ResourcePack.
   * @param name the name of this ImageResource.
   */
  ImageResource(const string& base_path, const string& name, const Align alignment) :
      base_path(base_path), image_name(name), alignment(alignment) { }

  /**
   * Reads this image resource into an OpenGL-compatible RGB(A) byte-array bitmap.
   * The caller is responsible for deallocating the returned byte array once finished.
   *
   * @param width OPTIONAL: a pointer to receive the decoded image's true width.
   * @param height OPTIONAL: a pointer to receive the decoded image's true height.
   * @param color_type OPTIONAL: a pointer to receive the decoded image's color type (usually RGBA).
   */
  png_byte* ReadAndDecode(int *width, int *height, int *color_type) const;

  /** Returns this ImageResource's name (without file extension). */
  string GetName() const { return this->image_name; }
  /** Returns this ImageResource's alignment. */
  Align GetAlignment() const { return this->alignment; }

  /** C++ enums are le suck. */
  static Align ParseAlignmentString(const string& align) {
    if (align == "left") {
      return Align::LEFT;
    } else if (align == "center") {
      return Align::CENTER;
    } else if (align == "right") {
      return Align::RIGHT;
    }
    return Align::CENTER;
  }

private:
  const string base_path;
  const string image_name;
  const Align alignment;
};

class AudioResource {
  DISALLOW_COPY_AND_ASSIGN(AudioResource)

  friend class ResourcePack;

public:

  enum class Beat {
    VERTICAL_BLUR,
    HORIZONTAL_BLUR,
    NO_BLUR,
    BLACKOUT,
    SHORT_BLACKOUT,
    COLOR_ONLY,
    IMAGE_ONLY,
    NO_TRANSITION
  };

  enum class Type {
    LOOP,
    BUILDUP
  };

  /**
   * Constructs a new, named AudioResource.
   *
   * @param base_path the base path of the containing ResourcePack.
   * @param name the name of this AudioResource.
   */
  AudioResource(const string& base_path, const string& song_title,
      const string& loop_name, const string& buildup_name) :
      buildup(buildup_name), loop(loop_name),
      base_path(base_path), song_title(song_title) { }

  /** Returns whether or not this song has a buildup. */
  bool HasBuildup() const { return !this->buildup.name.empty(); }

  string GetTitle() const { return song_title; }
  string GetName(const Type type) const {
    return (type == Type::LOOP ? this->loop : this->buildup).name;
  }
  string GetBeatmap(const Type type) const {
    return (type == Type::LOOP ? this->loop : this->buildup).beatmap;
  }
  double GetSongDurationUsec(const Type type) const {
    return (double) (type == Type::LOOP ? this->loop : this->buildup).sample_count
        / (type == Type::LOOP ? this->loop : this->buildup).sample_rate * 1000 * 1000;
  }
  double GetBeatDurationUsec(const Type type) const {
    return (type == Type::LOOP ? this->loop : this->buildup).usec_per_beat;
  }
  uint8_t* GetPcmData(const Type type) const {
    return (type == Type::LOOP ? this->loop : this->buildup).pcm_data;
  }
  size_t GetPcmDataSize(const Type type) const {
    return (type == Type::LOOP ? this->loop : this->buildup).sample_count
      * GetChannelCount(type) * /* bytes per sample */ 2;
  }
  int GetChannelCount(const Type type) const {
    return (type == Type::LOOP ? this->loop : this->buildup).channel_count;
  }
  int GetSampleRate(const Type type) const {
    return (type == Type::LOOP ? this->loop : this->buildup).sample_rate;
  }

  /**
   * Reads loop/buildup MP3 file (if present), decodes it into a 16 bits per channel, little-endian
   * PCM stream. Once decoded, the appropriate getters can be used to obtain information about this
   * audio resource.
   *
   * If called more than once for the same audio_type, nothing is done.
   *
   * @param audio_type controls whether we decode the loop or beatmap.
   */
  void ReadAndDecode(const Type audio_type);

  static Beat ParseBeatCharacter(const char beatChar) {
    switch (beatChar) {
      case 'x': return Beat::VERTICAL_BLUR;
      case 'o': return Beat::HORIZONTAL_BLUR;
      case '-': return Beat::NO_BLUR;
      case '+': return Beat::BLACKOUT;
      case '|': return Beat::SHORT_BLACKOUT;
      case ':': return Beat::COLOR_ONLY;
      case '*': return Beat::IMAGE_ONLY;
      case '.': return Beat::NO_TRANSITION;
    }
    return Beat::NO_TRANSITION;
  }

private:

  void SetBuildupBeatmap(const string& beatmap) { this->buildup.beatmap = beatmap; }
  void SetLoopBeatmap(const string& beatmap) { this->loop.beatmap = beatmap; }

  struct song_info {
    const string name;
    string beatmap;
    uint8_t *pcm_data;
    int channel_count;
    int sample_count;
    int sample_rate;
    double usec_per_beat;

    song_info(const string& name) : name(name) {}
  } buildup;
  struct song_info loop;

  const string base_path;
  const string song_title;
};

#endif // HUES_RESPACK_H_
