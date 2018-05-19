
static const unsigned char leftBwdPin = 5;
static const unsigned char leftFwdPin = 6;
static const unsigned char rightBwdPin = 9;
static const unsigned char rightFwdPin = 10;
static const unsigned char irPins[] = { 0, 0, 0, 0, 0 };
static const uint16_t irThreshold = 200;
static const uint32_t irSamplesCount = 255;
static const int maxDutyCycle = 255;


static void writeDutyCycle(unsigned char bwdPin, unsigned char fwdPin,
        int value);
static void writeLRDutyCycle(int left, int right);


void setup()
{
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


static bool hasLine(unsigned char i)
{
    static uint16_t last = analogRead(irPins[i]) < irThreshold;
    static uint32_t total = 0;
    static uint32_t count = 0;

    total += analogRead(irPins[i]);
    count++;

    if (count >= irSamplesCount) {
        last = total / count < irThreshold;
        total = count = 0;
    }

    return last;
}
