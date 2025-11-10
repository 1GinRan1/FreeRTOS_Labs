# FreeRTOS Labs

Welcome! This is  repository for laboratory work from the "Real-Time Operating Systems " course.

## Repository Structure

This repository is organized by the principle **"one branch per lab"**.

* The **`main`** branch (this branch) is "clean". It contains only this `README.md` file and a global `.gitignore`.
* Each laboratory assignment (`lab01`, `lab02`, etc.) is located in its **own separate branch** with the corresponding name.
* Within each branch (e.g., `lab01`), the lab files are located in their **own folder** (e.g., `lab01/`).

## Tools Used

* **IDE:** Visual Studio 2022 / Visual Studio Code
* **RTOS:** FreeRTOS (Windows Simulator)
* **Version Control:** Git

---

## General Build and Run Instructions

This repository contains **only the source code** for the labs, not the entire FreeRTOS project. To run any lab, you must paste these files into the FreeRTOS demo project.

### Step 1: Get the Lab Code

1.  Clone this repository (if you haven't already):
    ```bash
    git clone (https://github.com/1GinRan1/FreeRTOS_Labs.git)
    cd FreeRTOS_Labs
    ```

2.  Check out the branch for the lab you want to run. For example, for Lab 1:
    ```bash
    git checkout lab01
    ```

### Step 2: Configure the Visual Studio Project

1.  **Download and unzip** the original FreeRTOS project from the [official repository](https://github.com/FreeRTOS/FreeRTOS).

2.  Find and open the demo project in Visual Studio:
    `.../FreeRTOS/Demo/WIN32-MSVC/WIN32.sln`

3.  **Replace the files:**
    Copy the files from the `lab01` folder (in this repository) into the `.../WIN32-MSVC/` folder (where the `.sln` file is), **replacing** the original files:
    * `main.c`
    * `FreeRTOSConfig.h`

4.  **Add the new file to the project:**
    Important! Visual Studio does not know about your new `main_lab1.c` file. You must add it manually:
    * In Visual Studio, in the **"Solution Explorer"** window, right-click on the `Demo App Source` folder.
    * Select **Add** -> **Existing Item...**
    * Find and select your `main_lab1.c` file.

### Step 3: Build and Run

1.  In Visual Studio, click **Build** -> **Rebuild Solution** to ensure all changes are applied.
2.  Press **F5** or the green "Start" button to compile and run the project.
3.  A console window will appear showing the output of the lab.