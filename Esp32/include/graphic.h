
// void clearBG()
// {
//   gfx->fillTriangle(111, 0, 30, 120, 111, 240, GREENYELLOW);
//   gfx->fillTriangle(127, 0, 209, 120, 127, 240, GREENYELLOW);
//   gfx->fillRect(111, 0, 18, 240, GREENYELLOW);
// }



// void drawIdle(){


  
// }


// void drawMask()
// {
//   for (byte i = 0; i < 8; i++)
//   {
//     // Links Oben
//     gfx->drawLine(110 - i, 0, 29 - i, 120, BLACK);
//     // Recht Oben
//     gfx->drawLine(128 + i, 0, 210 + i, 120, BLACK);

//     // Linsk unten
//     gfx->drawLine(110 - i, 240, 29 - i, 121, BLACK);
//     //rechts unten
//     gfx->drawLine(128 + i, 240, 210 + i, 121, BLACK);
//   }
// }



// Pngle init callback
void pngleInitCallback(pngle_t *pngle, uint32_t w, uint32_t h)
{
  int16_t gfxW = gfx->width();
  int16_t gfxH = gfx->height();
//   xOffset = (w > gfxW) ? 0 : ((gfxW - w) / 2);
//   yOffset = (h > gfxH) ? 0 : ((gfxH - h) / 2);
// xOffset=65;
// yOffset=25;
}

// Pngle draw callback
void pngleDrawCallback(pngle_t *pngle, uint32_t x, uint32_t y, uint32_t w, uint32_t h, uint8_t rgba[4])
{
  if (rgba[3]) // not transparent
  {
    gfx->fillRect(x + xOffset, y + yOffset, w, h, gfx->color565(rgba[0], rgba[1], rgba[2]));
  }
}

void showImage(uint16_t fileNumber)
{
  String fileString = String(fileNumber);
  String fileName = "/" + fileString + ".png";
  // Serial.println(fileName);
  File pngFile = SPIFFS.open(fileName, "r");
  pngle_t *pngle = pngle_new();
  pngle_set_init_callback(pngle, pngleInitCallback);
  pngle_set_draw_callback(pngle, pngleDrawCallback);
  char buf[2048]; // buffer minimum size is 16 but it can be much larger, e.g. 2048
  int remain = 0;
  int len;
  //gfx->fillScreen(PINK); // transprant background color
  while ((len = pngFile.readBytes(buf + remain, sizeof(buf) - remain)) > 0)
  {
   
    int fed = pngle_feed(pngle, buf, remain + len);
    if (fed < 0)
    {
      Serial.printf("ERROR: %s\n", pngle_error(pngle));
      break;
    }
     

    remain = remain + len - fed;
    if (remain > 0)
    {
      memmove(buf, buf + fed, remain);
    }
  }

  pngle_destroy(pngle);
  pngFile.close();
}
