
static const unsigned char leftBwdPin = 5;
static const unsigned char leftFwdPin = 6;
static const unsigned char rightBwdPin = 9;
static const unsigned char rightFwdPin = 10;
static const unsigned char irPins[] = { A3, A0, A4, A1, A2 };
static const unsigned char clpPin = 2;
static const uint16_t irThreshold = 200;
static const uint32_t irSamplesCount = 5;
static const int maxDutyCycle = 255;
static const float lerpTime = 100;

static bool irLast[] = { false, false, false, false, false };
static uint32_t irValues[sizeof (irLast) / sizeof (*irLast)][irSamplesCount];
static uint32_t irCount = 0;
static bool irFilled = false;
static float targetLeft = 0;
static float targetRight = 0;
static float currentLeft = 0;
static float currentRight = 0;
static float startLeft = 0;
static float startRight = 0;
static unsigned long lerpStart = -1;


static void writeDutyCycle(unsigned char bwdPin, unsigned char fwdPin,
        int value);
static void writeLRDutyCycle(int left, int right);
static unsigned char readSensors();
static bool hasLine(unsigned char i);


void setup()
{
    Serial.begin(9600);
    pinMode(leftBwdPin, OUTPUT);
    pinMode(leftFwdPin, OUTPUT);
    pinMode(rightBwdPin, OUTPUT);
    pinMode(rightFwdPin, OUTPUT);
    pinMode(clpPin, INPUT_PULLUP);

    writeLRDutyCycle(0, 0);

    for (unsigned char i = 0; i < sizeof (irPins) / sizeof (*irPins); i++) {
        pinMode(irPins[i], INPUT);
    }
}


void loop()
{
    static unsigned char last = 0;
    static unsigned long lastTime = -1;

    if (digitalRead(clpPin)) {
        const int dutyCycle = maxDutyCycle / 2;

        writeLRDutyCycle(0, 0);
        delay(3000);
        writeLRDutyCycle(-dutyCycle, -dutyCycle);
        delay(4000);
        writeLRDutyCycle(0, 0);
    } 

    unsigned char current = readSensors();

    if (!current) {
        current = last;
    } 

    if (current & (1 << 2)) {
        lerpTo(maxDutyCycle, maxDutyCycle);
    } else if (current & (1 << 3)) {
        lerpTo(maxDutyCycle / 2, maxDutyCycle)
    } else if (current & (1 << 4)) {
        lerpTo(-maxDutyCycle, maxDutyCycle);
    } else if (current & (1 << 1)) {
        lerpTo(maxDutyCycle, maxDutyCycle / 2);
    } else if (current & 1) {
        lerpTo(maxDutyCycle, -maxDutyCycle);
    } else {
        lerpTo(maxDutyCycle, maxDutyCycle);
    }

    last = current;

    unsigned long diff = millis() - lerpStart;

    if (diff < lerpTime) {
        currentLeft = startLeft + (targetLeft - startLeft) * diff / lerpTime;
        currentRight = startRight + (targetRight - startRight) * diff /
            lerpTime;
    } else {
        currentLeft = targetLeft;
        currentRight = targetRight;
    }

//    Serial.print(currentLeft);
//    Serial.write(" ");
//    Serial.println(currentRight);
    writeLRDutyCycle(currentLeft, currentRight);
}


static void lerpTo(int left, int right)
{
    if (targetLeft == left && targetRight == right) {
        return;
    }

    targetLeft = left;
    targetRight = right;
    startLeft = currentLeft;
    startRight = currentRight;
    lerpStart = millis();
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


static unsigned char readSensors()
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

            irLast[i] = total / irSamplesCount > irThreshold;
        }
    }

    return (hasLine(0) << 4) | (hasLine(1) << 3) | (hasLine(2) << 2) |
           (hasLine(3) << 1) |  hasLine(4);
}


static bool hasLine(unsigned char i)
{
    return irLast[i];
}
