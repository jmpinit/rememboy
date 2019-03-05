import processing.serial.*;

int zoom = 8;

int x = 0;
int y = 0;

Serial port;

int[][] image = new int[128][128];

void settings() {
  size(128 * zoom, 128 * zoom);
}

void setup() {
  port = new Serial(this, "/dev/tty.usbmodem14101", 2000000);
}

void draw() {
  noStroke();
  for (int y = 0; y < 128; y++) {
    for (int x = 0; x < 128; x++) {
      fill(image[y][x]);
      rect(x * zoom, y * zoom, zoom, zoom);
    }
  }
}

void serialEvent(Serial port) {
  int v = port.read();
  
  image[y][x] = v;
  
  x += 1;
  
  if (x >= 128) {
    x = 0;
    y += 1;
    
    if (y >= 128) {
      y = 0;
    }
  }
}
