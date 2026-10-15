# NINMODS Automated Kernel Builder (GitHub Actions)

এই প্রজেক্টটির মাধ্যমে আপনার কম্পিউটারে কোনো ভারী ফাইল ডাউনলোড না করেই **GitHub-এর ফ্রি ক্লাউড সার্ভার** ব্যবহার করে ৯০%+ অ্যান্ড্রয়েড ফোনের কার্নেল ড্রাইভার (`.ko`) স্বয়ংক্রিয়ভাবে বিল্ড করতে পারবেন।

---

## 🎯 যে ৫টি কার্নেল ভার্সন বিল্ড হবে (৯০%+ ডিভাইস কভারেজ):
1. **Linux 4.14** (Android 9/10/11 — Helio G80/G85/G90, Snapdragon 665/720G/855, Infinix, Xiaomi ইত্যাদি)
2. **Linux 4.19** (Android 10/11/12 — Snapdragon 732G/750G, Dimensity 700/800U, Poco, Realme ইত্যাদি)
3. **Linux 5.4** (Android 11/12/13 — Snapdragon 870/888, Dimensity 1200)
4. **Linux 5.10** (Android 12/13 — GKI Devices, Snapdragon 8 Gen 1)
5. **Linux 5.15** (Android 13/14 — GKI 2.0 Devices, Snapdragon 8 Gen 2/3)

---

## 🚀 গিটহাবে আপলোড করে `.ko` ফাইল ডাউনলোড করার নিয়ম:

### ধাপ ১: GitHub-এ একটি নতুন রিপোজিটরি তৈরি করুন
1. [github.com](https://github.com)-এ যান এবং একটি নতুন রিপোজিটরি তৈরি করুন (যেমন নাম দিন: `kernel-driver-builder`)।
2. রিপোজিটরিটি Public অথবা Private যেকোনোটি রাখতে পারেন।

### ধাপ ২: এই ফোল্ডারটি গিটহাবে পুশ করুন
আপনার কম্পিউটারে টার্মিনাল খুলে এই কমান্ডগুলো চালান:

```bash
cd c:\Users\Sajala\Documents\mods\kernel_builder
git init
git add .
git commit -m "Add NINMODS Driver and GitHub Actions Workflow"
git branch -M main
git remote add origin https://github.com/<আপনার_ইউজারনেম>/kernel-driver-builder.git
git push -u origin main
```

### ধাপ ৩: গিটহাবের Actions ট্যাবে যান
1. গিটহাব রিপোজিটরির ওপরের মেনু থেকে **`Actions`** ট্যাবে ক্লিক করুন।
2. দেখতে পাবেন **"Build NINMODS Kernel Modules"** ওয়ার্কফ্লোটি অটোমেটিক রান হচ্ছে।
3. অথবা **`Run workflow`** বাটনে ক্লিক করে ম্যানুয়ালি স্টার্ট করতে পারেন।

### ধাপ ৪: কম্পাইল করা `.ko` ফাইল ডাউনলোড করুন
* মাত্র ২-৩ মিনিটের মধ্যে ৫টি কার্নেল ভার্সনের জন্যই পৃথকভাবে সম্পূর্ণ নতুন ও ফ্রেশ `.ko` ফাইল তৈরি হয়ে যাবে।
* Actions পেইজের নিচে **Artifacts** সেকশনে পাবেন:
  * `ninmods_driver_4.14.ko`
  * `ninmods_driver_4.19.ko`
  * `ninmods_driver_5.4.ko`
  * `ninmods_driver_5.10.ko`
  * `ninmods_driver_5.15.ko`
* সরাসরি ডাউনলোড করে জিপ ফাইলগুলো আনজিপ করে নিন।

---

## 📱 `driver_loder` অ্যাপে ফাইলগুলো কীভাবে বসাবেন:

ডাউনলোড করা ফাইলগুলো আমাদের `driver_loder`-এর এই ফোল্ডারে রেখে দিন:
```text
driver_loder/app/src/main/assets/kernels/android_11/4.14/ninmods_driver.ko
driver_loder/app/src/main/assets/kernels/android_11/4.19/ninmods_driver.ko
driver_loder/app/src/main/assets/kernels/android_12/5.4/ninmods_driver.ko
driver_loder/app/src/main/assets/kernels/android_12/5.10/ninmods_driver.ko
driver_loder/app/src/main/assets/kernels/android_13/5.15/ninmods_driver.ko
```
অথবা ফোনে মেমোরি কার্ড বা ইন্টারনাল স্টোরেজে এই পাথে রেখে দিলেও অ্যাপটি স্বয়ংক্রিয়ভাবে খুঁজে নেবে:
```text
/sdcard/NINMODS/kernels/
```
