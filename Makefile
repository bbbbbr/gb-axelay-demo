#
# A Makefile that compiles all .c and .s files in "src" and "res"
# subdirectories and places the output in a "obj" subdirectory
#

# If you move this project you can change the directory
# to match your GBDK root directory (ex: GBDK_HOME = "C:/GBDK/"
GBDK_HOME = ../../gbdk2020/gbdk-2020-git/build/gbdk/

# ~/git/Gimp/gimp-tilemap-gb/console/bin/linux/png2gbtiles horizon.png -csource -tileorg=245 horizon

LCC = $(GBDK_HOME)bin/lcc 
PNG2ASSET = $(GBDK_HOME)bin/png2asset

# You can set flags for LCC here
# For example, you can uncomment the line below to turn on debug output
# LCCFLAGS = -debug

# MBC5 + Ram + Battery, ROM Banks=0, RAM Banks = 2, DMG+CGB support
LCCFLAGS = -debug -Wl-yt0x1B -Wm-yn"GBAXELAY"  -Wl-ya1 -Wm-yS -Wm-yc

# MegaDuck
# LCCFLAGS = -debug -Wm-yn"GBAXELAY" -msm83:duck

# Analogue Pocket
# LCCFLAGS = -debug -Wl-yt0x1B -Wm-yn"GBAXELAY"  -Wl-ya1 -Wm-yS -Wm-yc -msm83:ap


# You can set the name of the .gb ROM file here
PROJECTNAME    = gbaxelay

SRCDIR      = src
OBJDIR      = obj
RESDIR      = res
BINS	    = $(OBJDIR)/$(PROJECTNAME).gb
# BINS	    = $(OBJDIR)/$(PROJECTNAME).duck
# BINS	    = $(OBJDIR)/$(PROJECTNAME).pocket
CSOURCES    = $(foreach dir,$(SRCDIR),$(notdir $(wildcard $(dir)/*.c))) $(foreach dir,$(RESDIR),$(notdir $(wildcard $(dir)/*.c)))
ASMSOURCES  = $(foreach dir,$(SRCDIR),$(notdir $(wildcard $(dir)/*.s)))
OBJS       = $(CSOURCES:%.c=$(OBJDIR)/%.o) $(ASMSOURCES:%.s=$(OBJDIR)/%.o)

all:	prepare $(BINS)

compile.bat: Makefile
	@echo "REM Automatically generated from Makefile" > compile.bat
	@make -sn | sed y/\\//\\\\/ | grep -v make >> compile.bat

# Compile .c files in "src/" to .o object files
$(OBJDIR)/%.o:	$(SRCDIR)/%.c
	$(LCC) $(LCCFLAGS) -c -o $@ $<

# Compile .c files in "res/" to .o object files
$(OBJDIR)/%.o:	$(RESDIR)/%.c
	$(LCC) $(LCCFLAGS) -c -o $@ $<

# Compile .s assembly files in "src/" to .o object files
$(OBJDIR)/%.o:	$(SRCDIR)/%.s
	$(LCC) $(LCCFLAGS) -c -o $@ $<

# If needed, compile .c files i n"src/" to .s assembly files
# (not required if .c is compiled directly to .o)
$(OBJDIR)/%.s:	$(SRCDIR)/%.c
	$(LCC) $(LCCFLAGS) -S -o $@ $<

# Link the compiled object files into a .gb ROM file
$(BINS):	$(OBJS)
	$(LCC) $(LCCFLAGS) -o $(BINS) $(OBJS)

prepare:
	mkdir -p $(OBJDIR)

assets:
#	== BG Map Conversion
#	tools/png2gbtiles res/nesaxelay_background_4_somelava_smaller.png -csource -c -f -p -pal=res/gb_axelay_rip_palette.txt -var=nes res/nes.c
#	tools/png2gbtiles res/nes_map.png -csource -c -f -p -pal=res/nes_map_palette.txt -var=nes res/nes.c
#    ---> TODO: Option to enable CGB palette array by default
#           nes_pal_cgb[]
	$(PNG2ASSET) res/nesaxelay_background_v2_indexed.png -map -use_map_attributes -keep_palette_order -c res/nesaxelay_background_map.c
#
#	== Sprite conversion
#    - use full 32 color indexed palette in Gimp, use mtpaint to fix gimps index color mangling if needed
#  - Boulders
	$(PNG2ASSET) res/sprite_boulders.png -sw 16 -sh 16 -px 0 -py 0 -pw 16 -ph 16 -spr8x16 -use_map_attributes -keep_palette_order -c res/sprite_boulders.c
#
#  - Ship
	$(PNG2ASSET) res/sprites_ship_nocanopy.png -sw 24 -sh 32 -px 0 -py 0 -pw 16 -ph 16  -spr8x16 -use_map_attributes -keep_palette_order -c res/sprite_ship.c
#
#  - Ship Canopy
	$(PNG2ASSET) res/sprites_ship_canopy-only_crop.png -sw 8 -sh 16 -px 0 -py 0 -pw 8 -ph 16  -spr8x16 -use_map_attributes -keep_palette_order -c res/sprite_ship_canopy.c
#
#  - Ship Bullets
	$(PNG2ASSET) res/sprites_shots.png -sw 8 -sh 16 -px 0 -py 0 -pw 8 -ph 16  -spr8x16 -use_map_attributes -keep_palette_order -c res/sprite_shots.c


clean:
#	rm -f  *.gb *.ihx *.cdb *.adb *.noi *.map
	rm -f  $(OBJDIR)/*.*

