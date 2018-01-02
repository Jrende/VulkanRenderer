#pragma once
struct QueueFamilyIndices {
  int graphics_family = -1;
  int present_family = -1;

  bool is_complete() const {
    return graphics_family >= 0 && present_family >= 0;
  }
};

