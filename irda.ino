
static const unsigned char leftBwdPin = 5;
static const unsigned char leftFwdPin = 6;
static const unsigned char rightBwdPin = 9;
static const unsigned char rightFwdPin = 10;
static const unsigned char irPins[] = { A0, A0, A0, A0, A0 };
static const uint16_t irThreshold = 200;
static const uint32_t irSamplesCount = 80;
static const int maxDutyCycle = 255;

static bool irLast[] = { false, false, false, false, false };
static uint32_t irValues[sizeof (irLast) / sizeof (*irLast)][irSamplesCount];
static uint32_t irCount = 0;
static bool irFilled = false;


static void writeDutyCycle(unsigned char bwdPin, unsigned char fwdPin,
        int value);
static void writeLRDutyCycle(int left, int right);
static void readSensors();
static bool hasLine(unsigned char i);


void setup()
{
    Serial.begin(9600);
    pinMode(leftBwdPin, OUTPUT);
    pinMode(leftFwdPin, OUTPUT);
    pinMode(rightBwdPin, OUTPUT);
    pinMode(rightFwdPin, OUTPUT);

    writeLRDutyCycle(0, 0);

    for (unsigned char i = 0; i < sizeof (irPins) / sizeof (*irPins); i++) {
        pinMode(irPins[i], INPUT);
    }
}


void loop()
{
    readSensors();

    if (hasLine(2)) {
        writeLRDutyCycle(maxDutyCycle, maxDutyCycle);
    } else if (hasLine(1)) {
        const int dutyCycle = maxDutyCycle / 2;

        writeLRDutyCycle(-dutyCycle, dutyCycle);
    } else if (hasLine(0)) {
        writeLRDutyCycle(-maxDutyCycle, maxDutyCycle);
    } else if (hasLine(3)) {
        const int dutyCycle = maxDutyCycle / 2;

        writeLRDutyCycle(dutyCycle, -dutyCycle);
    } else if (hasLine(4)) {
        writeLRDutyCycle(maxDutyCycle, -maxDutyCycle);
    } else {
        const int dutyCycle = maxDutyCycle / 4;

        writeLRDutyCycle(dutyCycle, dutyCycle);
    }
}


static void writeDutyCycle(unsigned char bwdPin, unsigned char fwdPin,
        int value)
{
    if (value > 0) {
        analogWrite(bwdPin, 0);
        analogWrite(fwdPin, value);
    } else {
        analogWrite(bwdPin, -value);
        analogWrite(fwdPin, 0);
    }
}


static void writeLRDutyCycle(int left, int right)
{
    writeDutyCycle(leftBwdPin, leftFwdPin, left);
    writeDutyCycle(rightBwdPin, rightFwdPin, right);
}


static void readSensors()
{
    for (unsigned char i = 0; i < sizeof (irPins) / sizeof (*irPins); i++) {
        irValues[i][irCount] = analogRead(irPins[i]);
    }

    irFilled = irFilled || (irCount >= irSamplesCount - 1);
    irCount = (irCount + 1) % irSamplesCount;

    if (irFilled) {
        for (unsigned char i = 0;
                i < sizeof (irPins) / sizeof (*irPins);
                i++) {

            uint32_t total = 0;

            for (uint32_t j = 0; j < irSamplesCount; j++) {
                total += irValues[i][j];
            }

            irLast[i] = total / irSamplesCount <= irThreshold;
        }
    }
}


static bool hasLine(unsigned char i)
{
    return irLast[i];
}
