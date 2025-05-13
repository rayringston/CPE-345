channel errorChannel extends ned.DatarateChannel {
    delay = 1ms;
    datarate = 1Mbps;
    ber = 0.00001;
}

simple Source
{
    parameters:
        @display("i=block/source");
        int limit = default(100000);

    gates:
        output out[];
}

simple User
{
    parameters:
        @display("i=block/server");
        double ber = default(0.1);
    gates:
        input in;
        output out;
}

simple Sink
{
    parameters:
        @display("i=block/sink");
    gates:
        input in[];
}

network Broadcast
{
    parameters:
        int count = default(5);

    submodules:
        source: Source {
            @display("p=100,500");
            gates: out[5];
        }
        
        users[count] : User {
            @display("p=300,500");
        }
        
        sink: Sink {
            @display("p=500,500");
            gates: in [5];
        }
    connections allowunconnected:
        for i=0..count-1 {
            source.out[i] --> errorChannel --> users[i].in;
            users[i].out --> sink.in[i];
        }
}
