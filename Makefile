CXX = g++

SRCDIR=./
COMMDIR=./common
NSDIR=./ns
AGCDIR=./agc
VADDIR=./vad
CCFILES += \
		$(SRCDIR)/main.c \
		$(SRCDIR)/front.c \
		$(COMMDIR)/copy_set_operations.c \
		$(COMMDIR)/division_operations.c \
		$(COMMDIR)/dot_product_with_scale.c \
		$(COMMDIR)/fft4g.c \
		$(COMMDIR)/resample_by_2.c \
		$(COMMDIR)/spl_sqrt.c \
		$(NSDIR)/noise_suppression.c \
		$(NSDIR)/ns_core.c \
		$(AGCDIR)/analog_agc.c \
		$(AGCDIR)/digital_agc.c

HFILES += \
		$(SRCDIR)/front.h \
		$(COMMDIR)/fft4g.h \
		$(COMMDIR)/signal_processing_library.h \
		$(COMMDIR)/spl_inl.h \
		$(COMMDIR)/typedefs.h \
		$(NSDIR)/defines.h \
		$(NSDIR)/noise_suppression.h \
		$(NSDIR)/ns_core.h \
		$(NSDIR)/windows_private.h \
		$(AGCDIR)/analog_agc.h \
		$(AGCDIR)/digital_agc.h \
		$(AGCDIR)/gain_control.h

LD_FLAGS         = -lm -lz

LD_LIBS          = 

OBJS             += $(patsubst %.c,%.c.o, $(CCFILES))

CFLAGS           =  -g -O0 -m64 -Wall

EXECUTABLE       = front

INCLUDE_FLAGS    = -I$(SRCDIR) -I$(COMMDIR) -I$(NSDIR) -I$(AGCDIR) -I$(VADDIR)

## Each subdirectory must supply rules for building sources it contributes
all: $(OBJS) $(CCFILES)
	$(CXX) -o $(EXECUTABLE) $(CFLAGS) $(OBJS) $(LD_FLAGS) 


$(OBJS): $(CCFILES) $(HFILES)
	$(CXX) $(CFLAGS) $(INCLUDE_FLAGS) -o $@ -c $*


.phony: clean


clean:
	@rm -f $(OBJS) $(EXECUTABLE)
