statetest:
	g++ -std=c++14 statetest.cpp Device.cpp Commands.cpp -I "../inc" -L "../lib" -liothub_client -liothub_client_amqp_transport -laziotsharedutil -luamqp -lm -lcurl -lssl -lcrypto -lpthread -o statetest

clean:
	rm statetest
