
PROTOS = keys.proto protocol.proto

PB_SRCS = $(PROTOS:.proto=.pb.cpp)
PB_HDRS = $(PROTOS:.proto=.pb.h)

all: $(PB_SRCS) $(PB_HDRS) keyconv_vk.cpp

clean:
	@rm -f $(PB_SRCS) $(PB_HDRS) keyconv_vk.cpp

keyconv_vk.cpp: keyconv_vk.cpp.erb | keycodes.yaml
	erb -T- $< <keycodes.yaml >$@
	
keys.proto: keys.proto.erb | keycodes.yaml
	erb -T- $< <keycodes.yaml >$@

# *.pb.cpp and *.pb.h are generated together by protoc
%.pb.h: %.pb.cpp	
%.pb.cpp: %.proto
	protoc --cpp_out=. $^

