module;

import std;
import star.data_stream;
import star.json;

namespace Star {

struct GameTimer {
  friend DataStream& operator>>(DataStream& ds, GameTimer& gt);
  friend DataStream& operator<<(DataStream& ds, GameTimer const& gt);
  GameTimer();
  explicit GameTimer(float time);

  float time;
  float timer;

  bool tick(float dt); // returns true if time is up
  bool wrapTick(float dt); // auto resets
  void reset();
  void setDone();
  void invert();

  bool ready() const;
  float percent() const;
};

DataStream& operator>>(DataStream& ds, GameTimer& gt);
DataStream& operator<<(DataStream& ds, GameTimer const& gt);

struct SlidingWindow {
  SlidingWindow();
  SlidingWindow(float windowSize, std::size_t resolution, float initialValue);

  GameTimer sampleTimer;
  float windowSize;
  std::size_t resolution;

  float currentMin;
  float currentMax;
  float currentAverage;

  std::size_t currentIndex;
  std::vector<float> window;

  void reset(float initialValue);
  void update(float dt, std::function<float()> sampleFunction);
  void update(float dt, float newValue);
  void processUpdate(float newValue);

  float min();
  float max();
  float average();
};

// Keeps long term track of elapsed time based on epochTime.
class EpochTimer {
public:
  EpochTimer();
  explicit EpochTimer(Json json);

  Json toJson() const;

  void update(double newEpochTime);

  double elapsedTime() const;
  void setElapsedTime(double elapsedTime);

  friend DataStream& operator>>(DataStream& ds, EpochTimer& et);
  friend DataStream& operator<<(DataStream& ds, EpochTimer const& et);

private:
  std::optional<double> m_lastSeenEpochTime;
  double m_elapsedTime;
};

}

export module star.game_timers;

export namespace Star {
  using ::Star::GameTimer;
  using ::Star::SlidingWindow;
  using ::Star::EpochTimer;
}
