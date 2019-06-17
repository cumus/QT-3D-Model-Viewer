# QT 3D Model Viewer
Authors: [Daniel Triviño](https://github.com/dibu13) and [Rubén Sardón](https://github.com/cumus/)

# What does it do?
The program loads a skyboxes and a sample model by default and dumps them to a scene with a camera the user can move. The way models render can be changed using the menu's tools.

## How to load/edit models
![Load Model Menu](https://cdn.discordapp.com/attachments/501422266152779776/590017055873302529/Captura04.PNG)

By default Patrick model is loaded. To add another model to the scene just select a file in the explorer and it will load together with its diffuse and specular texture if has any.

![Inspector](https://cdn.discordapp.com/attachments/501422266152779776/590015975684833280/Captura03.PNG)

All loaded models in the scene have a transform component through wich the model's position, rotation and size can be changed. The model can also toggle its Active check be drawn or discarded in the rendering process.

## How to enable / disable the implemented techniques
![Shaders Menu](https://cdn.discordapp.com/attachments/501422266152779776/590014836512129054/Captura01.PNG)

The way the models prints can be changed using the upper "Shaders" panel. From the list of options you can choose wich mode the shader will use to render the model. The 8 first options just draw the imported model data information. The next 2 will render the model either reflecting and refracting the skybox. Refraction index can be set for each model. Selected meshes will be marked using an orange border. Border drawing can be toggled on/off at the same menu.
![Skyboxes Menu](https://cdn.discordapp.com/attachments/501422266152779776/590015110039207946/Captura02.PNG)

By deafault, 2 different skyboxes load and it can be toggled wich one shows.
## Keyboard Controls
* WASD: move camera
* Q/E: move camera down/up
* Mouse Drag: pan camera
* Mouse Drag + Alt: orbit selected Model
* Mouse Wheel: zoom
* F: focus selected model

### Deferred mode light controls
* R: toggle camera light on/off
* T: toggle camera light follow/stay
* C: reset lights (turn all off)
* Space: drop light at camera position (up to 32 lights including camera's)

## Screenshots
Deferred Light Shading
![Deferred Shader](https://cdn.discordapp.com/attachments/501422266152779776/590024075582242817/Captura12_DeferredShader.PNG)
Reflection
![Reflection](https://cdn.discordapp.com/attachments/501422266152779776/590024836584046600/Captura13_Reflection.PNG)
Refraction
![Refraction](https://cdn.discordapp.com/attachments/501422266152779776/590024870956367872/Captura14_Refraction.PNG)
Borders
![Draw Borders](https://cdn.discordapp.com/attachments/501422266152779776/590025306488963083/Captura15_DrawBorders.PNG)
Secondary Skybox
![Skybox](https://cdn.discordapp.com/attachments/501422266152779776/590026324039696394/Captura16_Skybox.PNG)
Vertex Position
![Position](https://cdn.discordapp.com/attachments/501422266152779776/590017890111979531/Captura05_VertexPosition.PNG)
Vertex Normal
![Normal](https://cdn.discordapp.com/attachments/501422266152779776/590018101437792326/Captura06_VertexNormal.PNG)
Vertex Texture Coord
![Texture Coord](https://cdn.discordapp.com/attachments/501422266152779776/590018303385141279/Captura07_VertexTextCoord.PNG)
Vertex Bitangent
![Bitangents](https://cdn.discordapp.com/attachments/501422266152779776/590018489440534551/Captura08_Bitangents.PNG)
Vertex Tangents
![Tangents](https://cdn.discordapp.com/attachments/501422266152779776/590018671368208429/Captura09_Tangents.PNG)
Depth
![Depth](https://cdn.discordapp.com/attachments/501422266152779776/590018891413979165/Captura10_Depth.PNG)
Linear Depth
![Linear Depth](https://cdn.discordapp.com/attachments/501422266152779776/590019063149887488/Captura11_LinearDepth.PNG)

