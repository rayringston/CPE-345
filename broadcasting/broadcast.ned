channel errorChannel extends ned.DatarateChannel {
    delay = 1ms;
    datarate = 1Mbps;
    ber = 0.00001;
}

channel userChannel extends ned.DatarateChannel {
    datarate = 1Mbps;
    ber = 0.000005;
}

simple Source {
    parameters:
        @display("i=block/source");
        int limit = default(100000);
        double interArrivalTime @unit(s) = default(0.01s);
    gates:
        output out[];
}

simple User {
    parameters:
        @display("i=block/server");
    gates:
        input in;
        output out;
}

simple Sink {
    parameters:
        @display("i=block/sink");
    gates:
        input in[];
}

network Broadcast {
    parameters:
        int count = default(5);

    submodules:
        source: Source {
            @display("p=100,500");
            gates: out[parent.count]; //<------recent error
        }

        users[count]: User {
            @display("p=300,500");
        }

        sink: Sink {
            @display("p=500,500");
            gates: in[parent.count];
        }

    connections allowunconnected:
        for i=0..count-1 {
            source.out[i] --> errorChannel --> users[i].in;
            users[i].out --> sink.in[i];
        }
}

network Sequential {
    parameters:
        int count = default(5);

    submodules:
        source: Source {
            @display("p=100,500");
            gates: out[1];
        }

        users[count]: User {
            @display("p=200+100*i,500");
        }

       sink: Sink {
    @display("p=800,500");
    gates: in[1];  
	}

    connections allowunconnected:
        source.out[0] --> errorChannel --> users[0].in;

        for i = 0..count - 2 {
            users[i].out --> userChannel --> users[i + 1].in;
        }

        users[count - 1].out --> sink.in[0];
}
