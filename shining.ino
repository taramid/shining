const byte SOUND_PIN = 3;
const byte SENSOR_PIN = 5;
const byte LIGHT_PIN = 7;

const unsigned long LIGHT_TIMEOUT = 180000;
const unsigned long BEEP_INTERVAL = 60000;


//---------------------------------------------------------- Sound {}
struct Beep {
  word tone;
  word duration;
  word gap;

  constexpr Beep(word t, word d, word g = 0) : tone(t), duration(d), gap(g) {}
};
class Sound {

  inline static constexpr Beep Interval[] = {
    {2020, 80, 144},
  };

  inline static constexpr Beep Start[] = {
    {244, 44},
  };

  inline static constexpr Beep Stop[] = {
    {66, 200},
    {44, 100},
  };

  byte pin;

  public:
    Sound(byte pin) {
      pinMode(pin, OUTPUT);
      this->pin = pin;
    }

    void onStart() {
      this->play(Start);
    }

    void interval(word n) {
      for (word i=0; i<n; ++i) {
        this->play(Interval);
      }
    }

    void onStop() {
      this->play(Stop);
    }

  private:
    void beep(Beep beep) {
      tone(pin, beep.tone, beep.duration);
      delay(beep.duration + beep.gap);
    }

    template <size_t N>
    void play(const Beep (&melody)[N]) {
      for (size_t i = 0; i < N; ++i) {
        beep(melody[i]);
      }
    }
};

Sound *sound = NULL;


//---------------------------------------------------------- Motion {}
class Motion {

  byte pin;

  public:
    Motion(byte pin) : pin(pin) {
      pinMode(pin, INPUT);
    }

    bool is() {
      return HIGH == digitalRead(this->pin);
    }
};

Motion *motion = NULL;


//---------------------------------------------------------- Light {}
class Light {

  byte pin;
  unsigned long timeout;
  unsigned long started;
  bool isON = false;

  Sound *sound;

  public:
    Light(byte pin, unsigned long timeout, Sound *sound) : pin(pin), timeout(timeout), sound(sound) {

      pinMode(pin, OUTPUT);
      update();
    }

    void on() {
      started = millis();

      if (!isON) {

        isON = true;
        update();

        sound->onStart();
      }
    }

    bool tick() {
      if (isON && (millis() - started >= timeout)) {

        isON = false;
        update();

        sound->onStop();
      }

      return isON;
      // Serial.print(this->timeout);
      // Serial.print(" : ");
      // Serial.println(millis() - this->started);
    }

  private:
    void update() {
      digitalWrite(pin, isON);
    }
};

Light *light = NULL;



class Tracker {

  unsigned long interval;
  unsigned long started;

  Sound *sound;

  word n;

  public:
    Tracker(unsigned long interval, Sound *sound) : interval(interval), sound(sound) {}
    
    void go() {
      if (0 == started) {
        started = millis();
        n = 0;
      }

      if (millis() - started - n * interval >= interval) {
        sound->interval(++n);
        // Serial.println(n);
      }
    }

    void stop() {
      this->started = 0;
    }

};

Tracker *tracker = NULL;


void setup() {

  // Serial.begin(9600);

  sound = new Sound(SOUND_PIN);
  motion = new Motion(SENSOR_PIN);

  light = new Light(LIGHT_PIN, LIGHT_TIMEOUT, sound);

  tracker = new Tracker(BEEP_INTERVAL, sound);
}


void loop() {

  if (motion->is()) {
    light->on();
  }

  if (light->tick()) {
    tracker->go();
  } else {
    tracker->stop();
  }
}
