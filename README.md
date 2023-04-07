# VTX
<h3>A procedural universe generation program</h3>

VTX stands for "Virtual Terrain EXplorer"

The program is designed to generate an artificial Universe containing galaxies, Stars, Planets, Moons etc.
with an arbitrary level of detail using primarily procedural techniques.

The VTX project is based on WxWidgets for the UI, OpenGL for the graphics and C++ for the programming language.
It was designed to be cross-platform and able to build and run on Windows, Linux and (presumably)
Apple operating systems (although a port to the Mac has not been tested)

The program utilizes a custom text-based description language that allows user generated scenes to be 
created and modified via a standard text editor or by using the extensive graphical editing 
interface available in the application

Some screenshots of previously generated scenes are available in the Images sub-directory

<h2>Getting Started</h2>
<h3>System Requirements</h3>
<ol>
<li>Desktop or laptop computer</li>
  <ul>
    <li>memory: 8GB+</li>
    <li>graphics card: NVIDIA 980(m) or better (note: built in Intel graphics processor will not work)</li>
  </ul>
<li>Operating Sytem (tested)</li>
  <ul>
    <li>Windows 10 or 11 </li>
    <li>Ubuntu 18.04</li>
   </ul>
<li>Other Software</li>
  <ul>
    <li>Gitbash terminal</li>
  </ul>
</ol>
<h3>Running the program(using the files in this repository</h3>
<ol>
  <li>Executing VTX</li>
    <ul>
      <li>Open a Gitbash shell</li>
      <li>clone this repository into a computer directory of your choice (e.g. git clone https://github.com/dwsindorf/VTX)
      <li>"cd" to the "binaries" directory of this project</li>
      <li>launch the program</li>
      <ul>
        <li>Windows: ./vtx</li>
        <li>Linux: ./vtx-linux</li>
      </ul>
      <li>if successful you should see the following program startup window</li>
      <img src="https://github.com/dwsindorf/VTX/blob/master/Resources/Screenshots/VTX-Startup-window.jpg">
   </ul>
  <li>Navigating to a new Random Star system</li>
  <ul>
    <li>Select a star in the star field and Press the "goto orbit" button in the menu bar</li>
    <img src="https://github.com/dwsindorf/VTX/blob/master/Resources/Screenshots/select-star-and-go-to-orbit.jpg">
    <li>open the Scene tree dialog window</li>
    <img src="https://github.com/dwsindorf/VTX/blob/master/Resources/Screenshots/new-star-system-view.jpg">
    <li>Select a system object to view</li>
    <img src="https://github.com/dwsindorf/VTX/blob/master/Resources/Screenshots/scenetree-dialog-set-view.jpg">
    <li>View/edit selected object</li>
    <img src="https://github.com/dwsindorf/VTX/blob/master/Resources/Screenshots/new-random-gas-giant.png">
    <li>This procedure can be repeated again if desired by selecting a new star from the generated system ..</li>
  </ul>
  </ul>
 </ol>



