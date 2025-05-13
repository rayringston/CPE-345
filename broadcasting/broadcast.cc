#include <omnetpp.h>

using namespace omnetpp;

// Source module - sends packets up to the limit
class Source : public cSimpleModule {
  private:
    simtime_t timeout;
    cMessage *timeoutEvent;
    int limit;
    int sentCount = 0;

  public:
    Source();
    virtual ~Source();

  protected:
    virtual void initialize() override;
    virtual void handleMessage(cMessage *message) override;
};

Define_Module(Source);

Source::Source() {
    timeoutEvent = nullptr;
}

Source::~Source() {
    cancelAndDelete(timeoutEvent);
}

void Source::initialize() {
    timeout = 1.0;  // Time between sending packets (1s)
    limit = par("limit");  // Total number of packets to send
    timeoutEvent = new cMessage("timeoutEvent");

    // Schedule the first packet to be sent
    scheduleAt(simTime(), timeoutEvent);
}

void Source::handleMessage(cMessage *msg) {
    if (msg == timeoutEvent) {
        if (sentCount < limit) {
            cPacket *pkt = new cPacket("packet");

            int size = intuniform(1, 160); // sms text messages can be anywhere between 1 and 160 bytes
            pkt->setBitLength(size * 8);
            pkt->setTimestamp();

            EV << "Sending packet " << sentCount + 1 << " of " << limit << "\n";

            int numGates = gateSize("out"); // finds the number of gates

            for (int i = 0; i < numGates; i++) {
                send(pkt->dup(), "out", i);
            }

            sentCount++;

            // Schedule the next packet after the timeout
            scheduleAt(simTime() + timeout, timeoutEvent);
        } else {
            EV << "Reached packet limit. Ending simulation.\n";
            endSimulation();
        }
    }
}

// User module - Receives packets, calculates delay, and forwards them to the sink
class User : public cSimpleModule {
  private:
    cStdDev delayStat;
    cHistogram delayHist;

  protected:
    virtual void handleMessage(cMessage *msg) override;
    virtual void finish() override;
};

Define_Module(User);

void User::handleMessage(cMessage *msg) {
    cPacket *pkt = check_and_cast<cPacket *>(msg);

    // Simulate some packet loss based on Bit Error Rate (BER)
    if (pkt->hasBitError()) {
        EV << "Packet lost due to bit error.\n";
        bubble("Packet dropped");
        delete pkt;
    } else {
        // Calculate delay and collect statistics
        simtime_t delay = simTime() - pkt->getTimestamp();
        delayStat.collect(delay);
        delayHist.collect(delay);

        EV << "Packet received with delay: " << delay << "\n";

        // Forward the packet to the next module (sink)
        send(pkt, "out");
    }
}

void User::finish() {
    recordScalar("Mean delay", delayStat.getMean());
    recordScalar("Stddev", delayStat.getStddev());
    recordScalar("Variance", delayStat.getVariance());
    delayHist.record();
}

// Sink module - Receives packets and logs final statistics
class Sink : public cSimpleModule {
  private:
    cStdDev delayStat;
    cHistogram delayHist;

  protected:
    virtual void handleMessage(cMessage *msg) override;
    virtual void finish() override;
};

Define_Module(Sink);

void Sink::handleMessage(cMessage *msg) {
    cPacket *pkt = check_and_cast<cPacket *>(msg);

    // Log delay statistics when packet reaches the sink
    simtime_t delay = simTime() - pkt->getTimestamp();
    delayStat.collect(delay);
    delayHist.collect(delay);

    EV << "Packet received at sink with delay: " << delay << "\n";

    delete pkt;  // Sink does not forward packets
}

void Sink::finish() {
    recordScalar("Mean delay", delayStat.getMean());
    recordScalar("Stddev", delayStat.getStddev());
    recordScalar("Variance", delayStat.getVariance());
    delayHist.record();
}
