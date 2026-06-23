const byte BEEPER_PIN = 3;
const byte SENSOR_PIN = 5;
const byte LIGHT_PIN = 7;

const unsigned long LIGHT_TIMEOUT = 180000;
const unsigned long BEEP_INTERVAL = 60000;


//---------------------------------------------------------- Beeper {}
class Beeper {

  struct Beep {
    word tone;
    word duration;
    word gap;
  };

  inline static constexpr Beep Interval = {2020, 80, 200};
  inline static constexpr Beep Start    = {555, 100, 0};
  inline static constexpr Beep Stop     = {88, 400, 0};

  byte pin;

  public:
    Beeper(byte pin) {
      pinMode(pin, OUTPUT);
      this->pin = pin;
    }

    void onStart() {
      this->beep(Start);
    }

    void interval(word n) {
      for (word i=0; i<n; ++i) {
        this->beep(Interval);
      }
    }

    void onStop() {
      this->beep(Stop);
    }

  private:
    void beep( Beep beep) {
      tone(pin, beep.tone, beep.duration);
      if (0 < beep.gap) {
        delay(beep.gap);
      }
    }
};

Beeper *beeper = NULL;


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

Motion *sensor = NULL;


//---------------------------------------------------------- Light {}
class Light {

  byte pin;
  unsigned long timeout;
  unsigned long started;
  bool isON = false;

  Beeper *beeper;

  public:
    Light(byte pin, unsigned long timeout, Beeper *beeper) : pin(pin), timeout(timeout), beeper(beeper) {

      pinMode(pin, OUTPUT);
      digitalWrite(pin, isON);
    }

    void on() {
      started = millis();

      if (!isON) {
        isON = true;
        update();

        beeper->onStart();
      }
    }

    bool tick() {
      if (isON && (millis() - started >= timeout)) {

        isON = false;
        update();

        beeper->onStop();
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

  Beeper *beeper;

  word n;

  public:
    Tracker(unsigned long interval, Beeper *beeper) : interval(interval), beeper(beeper) {}
    
    void go() {
      if (0 == started) {
        started = millis();
        n = 0;
      }

      if (millis() - started - n * interval >= interval) {
        beeper->interval(++n);
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

  beeper = new Beeper(BEEPER_PIN);
  sensor = new Motion(SENSOR_PIN);

  light = new Light(LIGHT_PIN, LIGHT_TIMEOUT, beeper);

  tracker = new Tracker(BEEP_INTERVAL, beeper);
}


void loop() {

  if (sensor->is()) {
    light->on();
  }

  if (light->tick()) {
    tracker->go();
  } else {
    tracker->stop();
  }
}
