#include "raylib.h"
#include "include/raymath.h"
#include <stdio.h>
#include <string.h>

#define RAYLIB_NUKLEAR_IMPLEMENTATION
#include "include/raylib-nuklear.h"
#define RLIGHTS_IMPLEMENTATION
#include "include/rlights.h"
#define GLSL_VERSION 330

#define DEBUG_BACKGROUND \
    CLITERAL(Color){0, 0, 0, 150}
#define MAX_PARTICLE 300

typedef struct Particle
{
    float x;
    float y;
    float z;
    float lifetime;
    bool active;
} Particle;
float RandomFloat(float min, float max)
{
    return min + (float)GetRandomValue(0, 10000) / 10000.0f * (max - min);
}
Particle particle[MAX_PARTICLE] = {0};

void getScaledBoundingBox(BoundingBox *box, Model model, Vector3 position, Vector3 scale)
{
    BoundingBox rawBox = GetModelBoundingBox(model);
    box->min = Vector3Add(Vector3Multiply(rawBox.min, scale), position);
    box->max = Vector3Add(Vector3Multiply(rawBox.max, scale), position);
}
void DrawTextLetterByLetter(Font font, const char *text, int x, int y, int fontSize, float spacing, Color color, float letterDelay)
{
    static int charIndex = 0;
    static float timer = 0.0f;
    static int TextDisplayWhenEnd = 120; // Time to display the full text after the last character is drawn

    timer += GetFrameTime();

    // Draw the text up to the current character index
    char temp[256] = {0}; // Adjust size as needed
    strncpy(temp, text, charIndex);
    temp[charIndex] = '\0';
    if (timer >= letterDelay && charIndex < strlen(text))
    {
        timer -= letterDelay;
        charIndex++;
        DrawTextEx(font, temp, (Vector2){x, y}, fontSize, spacing, color);
    }
    else
    {
        TextDisplayWhenEnd--;
        if (TextDisplayWhenEnd >= 0)
        {
            DrawTextEx(font, text, (Vector2){x, y}, fontSize, spacing, color);
        }
        else
        {
            TextDisplayWhenEnd = 0;
        }
    }
    // printf("TextDisplayWhenEnd: %d\n", TextDisplayWhenEnd);
}
void DrawFpsEx(Font font, Vector2 position, float fontsize, float spacing)
{
    Color color = LIME;
    int fps = GetFPS();
    DrawTextEx(font, TextFormat("%2i FPS", fps), position, fontsize, 3, color);
}
void DrawTimer(Font font)
{
    static float timer = 0.0f;
    static int seconds = 0;
    static int minutes = 0;
    static int hours = 0;

    float deltaTime = GetFrameTime();
    timer += deltaTime;

    if (timer >= 1.0f)
    {
        timer -= 1.0f;
        seconds++;

        if (seconds >= 60)
        {
            seconds = 0;
            minutes++;

            if (minutes >= 60)
            {
                minutes = 0;
                hours++;

                if (hours >= 24)
                    hours = 0;
            }
        }
    }
    DrawTextEx(font, TextFormat("%02i:%02i:%02i", hours, minutes, seconds), (Vector2){800, 10}, 20, 2, WHITE);
}
void SpawnBillboardEntity(Camera camera, Vector3 position, Texture2D tex, float ObjectScale, Color color)
{
    static Vector3 particlePositions[MAX_PARTICLE];
    for (int i = 0; i < MAX_PARTICLE; i++)
    {
        float randX = RandomFloat(position.x - 3.0f, position.x + 3.0f);
        float randY = RandomFloat(position.y - 3.0f, position.y + 3.0f);
        float randZ = RandomFloat(position.z - 3.0f, position.z + 3.0f);
        particlePositions[i] = (Vector3){randX, randY, randZ};
        DrawBillboard(camera, tex, particlePositions[i], ObjectScale, color);
    }
}

void CustomNuklearStyle(struct nk_context *ctx)
{
    struct nk_style *style = &ctx->style;

    style->window.border_color = nk_rgb(180, 0, 0);
    style->window.border = 2.0f;
    style->window.background = nk_rgb(255, 50, 50);

    style->button.normal = nk_style_item_color(nk_rgb(50, 50, 50));
    style->button.hover = nk_style_item_color(nk_rgb(90, 10, 10));
    style->button.active = nk_style_item_color(nk_rgb(150, 30, 30));
    style->button.border_color = nk_rgb(220, 220, 220);
    style->button.border = 1.0f;

    style->text.color = nk_rgb(220, 220, 220);

    style->window.padding = nk_vec2(15, 15);
    style->button.padding = nk_vec2(10, 5);
}

void CreateParticle(Camera camera, Vector3 Pos, Shader shader, float maxHeight, Color beginColor, Color middleColor, Color endColor)
{
    Color ColorHolder = {0, 0, 0, 0};
    static Vector3 particlePositions[MAX_PARTICLE];
    static bool initialized = false;
    CreateLight(LIGHT_POINT, Pos, Vector3Zero(), beginColor, shader);
    DrawSphereEx(Pos, 0.2f, 8, 8, beginColor);

    if (!initialized)
    {
        for (int i = 0; i < MAX_PARTICLE; i++)
        {
            float randX = RandomFloat(Pos.x - 1.0f, Pos.x + 1.0f);
            float randY = RandomFloat(Pos.y - 1.0f, Pos.y + maxHeight);
            float randZ = RandomFloat(Pos.z - 1.0f, Pos.z + 1.0f);
            particlePositions[i] = (Vector3){randX, randY, randZ};
        }
        initialized = true;
    }

    for (int i = 0; i < MAX_PARTICLE; i++)
    {
        if (particlePositions[i].y <= maxHeight / 3)
        {
            ColorHolder = beginColor;
        }
        else if (particlePositions[i].y <= (maxHeight / 3) * 2)
        {
            ColorHolder = middleColor;
        }
        else
        {
            ColorHolder = endColor;
        }
        DrawCube(particlePositions[i], 0.2f, 0.2f, 0.2f, ColorHolder);

        // int randParticle = GetRandomValue(0, MAX_PARTICLE);
        // particlePositions[randParticle].y += 0.05f;
        particlePositions[i].y += 0.05f;

        if (particlePositions[i].y >= Pos.y + maxHeight)
        {
            particlePositions[i].y = Pos.y;
        }
    }
}

void zoomControl(Camera *camera, float maxFOV, Vector3 *shotgunOffset)
{
    // FOV control
    if (IsKeyDown(KEY_X) && camera->fovy >= 20.0f)
    {
        camera->fovy -= 2.0f;
        shotgunOffset->z += 0.02f;
    }
    else if (IsKeyDown(KEY_C) && camera->fovy <= maxFOV)
    {
        camera->fovy += 2.0f;
        shotgunOffset->z -= 0.02f;
    }
}

int main()
{
    Image icon = LoadImage("icon.png");
    SetConfigFlags(FLAG_MSAA_4X_HINT);
    int screenWidth = 1600;
    int screenHeight = 900;
    InitWindow(screenWidth, screenHeight, "raylib");
    SetWindowIcon(icon);

    // ####################################################################################################
    //                                   SHADER AND POSTPROCESSING PART
    // ####################################################################################################
    Shader shader = LoadShader(TextFormat("resources/shaders/lighting.vs", GLSL_VERSION),
                               TextFormat("resources/shaders/lighting.fs", GLSL_VERSION));
    // Get some required shader locations
    shader.locs[SHADER_LOC_VECTOR_VIEW] = GetShaderLocation(shader, "viewPos");

    // Ambient light level (some basic lighting)
    int ambientLoc = GetShaderLocation(shader, "ambient");

    SetShaderValue(shader, ambientLoc, (float[4]){0.1f, 0.1f, 0.1f, 1.0f}, SHADER_UNIFORM_VEC4);

    // Create lights
    Light lights[MAX_LIGHTS] = {0};
    lights[0] = CreateLight(LIGHT_POINT, (Vector3){0, 6, 0}, Vector3Zero(), WHITE, shader);
    // lights[1] = CreateLight(LIGHT_POINT, (Vector3){ 6, 6, 0 }, Vector3Zero(), RED, shader);
    // lights[2] = CreateLight(LIGHT_POINT, (Vector3){ 0, 6, 6 }, Vector3Zero(), GREEN, shader);
    // lights[3] = CreateLight(LIGHT_POINT, (Vector3){ 6, 6, 6 }, Vector3Zero(), YELLOW, shader);

    Shader shader_pix = LoadShader(0, TextFormat("resources/shaders/pixelizer.fs", GLSL_VERSION));
    //  Uncomment the line below to Disable pixel shader
    // Shader shader_pix = LoadShader(0, TextFormat("ressources/shaders/pixelizer.fs", GLSL_VERSION));
    RenderTexture2D shader_tex = LoadRenderTexture(screenWidth, screenHeight);

    // ####################################################################################################
    //                                    VARIABLES AND MODELS LOADER
    // ####################################################################################################

    const float groundLevel = 1.4f;
    float maxFOV = 90.0f;
    Vector3 PlayerPos = {10.5f, groundLevel, 10.5f};
    Vector3 PlayerHitboxMin = {10.0f, PlayerPos.y - groundLevel, 10.0f};
    Vector3 PlayerHitboxMax = {11.0f, groundLevel + 0.6f, 11.0f};

    Camera3D camera = {
        .position = PlayerPos,
        .target = {0.0f, 0.0f, 0.0f},
        .up = {0.0f, 1.0f, 0.0f},
        .fovy = maxFOV,
        .projection = CAMERA_PERSPECTIVE};

    Camera3D weaponCamera = {
        .position = PlayerPos,
        .target = {0.0f, 0.0f, -100.0f},
        .up = {0.0f, 1.0f, 0.0f},
        .fovy = maxFOV,
        .projection = CAMERA_PERSPECTIVE};

    Font font = LoadFontEx("resources/font/PressStart2P-Regular.ttf", 18, NULL, 0);

    Model dingusMesh = LoadModel("resources/models/mesh/dingus.obj");
    Texture2D dingus_tex = LoadTexture("resources/models/textures/bsc5.png");
    SetMaterialTexture(&dingusMesh.materials[0], MATERIAL_MAP_DIFFUSE, dingus_tex);
    dingusMesh.materials[0].shader = shader;

    Vector3 dingusPos = {2.4f, 0.8f, 3.0f};
    Vector3 dingusRot = {1.0f, 0.0f, 0.0f};
    Vector3 dingusScale = {0.01f, 0.01f, 0.01f};
    int dingusHealth = 1;

    Model tableMesh = LoadModel("resources/models/mesh/table.obj");
    Texture2D table_tex = LoadTexture("resources/models/textures/table.png");
    SetMaterialTexture(&tableMesh.materials[0], MATERIAL_MAP_DIFFUSE, table_tex);
    tableMesh.materials[0].shader = shader;

    Vector3 tablePos = {3.0f, 0.45f, 3.0f};
    Vector3 tableRot = {1.0f, 0.0f, 0.0f};
    Vector3 tableScale = {0.01f, 0.01f, 0.01f};

    Model pcMesh = LoadModel("resources/models/mesh/Pc.obj");
    Texture2D pc_tex = LoadTexture("resources/models/textures/Pc.png");
    SetMaterialTexture(&pcMesh.materials[0], MATERIAL_MAP_DIFFUSE, pc_tex);
    pcMesh.materials[0].shader = shader;

    Vector3 pcPos = {3.5f, 0.84f, 3.0f};
    Vector3 pcRot = {1.0F, 0.0F, 0.0f};
    Vector3 pcScale = {0.01f, 0.01f, 0.01f};

    Model carMesh = LoadModel("resources/models/mesh/ALESTA.obj");
    Texture2D car_tex = LoadTexture("resources/models/textures/ALESTA.png");
    SetMaterialTexture(&carMesh.materials[0], MATERIAL_MAP_DIFFUSE, car_tex);
    carMesh.materials[0].shader = shader;

    Vector3 carPos = {8.0f, 0.0f, 3.0f};
    Vector3 carRot = {1.0F, 0.0F, 0.0f};
    Vector3 carScale = {1.3f, 1.3f, 1.3f};

    Model shotgunMesh = LoadModel("resources/models/mesh/shotgun.obj");
    Texture2D shotgun_tex = LoadTexture("resources/models/textures/shotgun_tex.jpg");
    SetMaterialTexture(&shotgunMesh.materials[0], MATERIAL_MAP_DIFFUSE, shotgun_tex);
    shotgunMesh.materials[0].shader = shader;

    Vector3 shotgunPos = {5.0f, 0.3f, 2.0f};
    Vector3 shotgunRot = {1.0F, 0.0F, 0.0f};
    Vector3 shotgunScale = {0.1f, 0.1f, 0.1f};

    Model ammoBoxMesh = LoadModel("resources/models/mesh/ammo_shell.obj");
    Texture2D ammoBox_tex = LoadTexture("resources/models/textures/ammo_shell.png");
    SetMaterialTexture(&ammoBoxMesh.materials[0], MATERIAL_MAP_DIFFUSE, ammoBox_tex);
    ammoBoxMesh.materials[0].shader = shader;

    Vector3 ammoBoxPos = {2.1f, 0.95f, 3.0f};
    Vector3 ammoBoxRot = {0.0f, 1.0f, 0.0f};
    Vector3 ammoBoxScale = {0.2f, 0.2f, 0.2f};

    Model boxMesh = LoadModel("resources/models/mesh/little_box.obj");
    Texture2D box_tex = LoadTexture("resources/models/textures/little_box.png");
    SetMaterialTexture(&boxMesh.materials[0], MATERIAL_MAP_DIFFUSE, box_tex);
    boxMesh.materials[0].shader = shader;

    Vector3 boxPos = {0.0f, 0.0f, 0.0f};
    Vector3 boxRot = {1.0f, 0.0f, 0.0f};
    Vector3 boxScale = {0.5f, 0.5f, 0.5f};

    Model npcMesh = LoadModel("resources/models/mesh/Npc.obj");
    Texture2D npc_tex = LoadTexture("resources/models/textures/Npc.png");
    SetMaterialTexture(&npcMesh.materials[0], MATERIAL_MAP_DIFFUSE, npc_tex);
    npcMesh.materials[0].shader = shader;

    Vector3 npcPos = {10.0f, 1.07f, -10.0f};
    Vector3 npcRot = {1.0f, 0.0f, 0.0f};
    Vector3 npcScale = {0.018f, 0.018f, 0.018f};

    Texture2D blood_tex = LoadTexture("resources/textures/Blood.png");
    Mesh planeMesh = GenMeshPlane(1.0f, 1.0f, 1, 1);
    Model bloodMesh = LoadModelFromMesh(planeMesh);
    SetMaterialTexture(&bloodMesh.materials[0], MATERIAL_MAP_DIFFUSE, blood_tex);
    bloodMesh.materials[0].shader = shader;

    //----------------GROUND-------------------
    // Generate a procedural chessboard texture
    Image gen_ground_image = GenImageChecked(8192, 8192, 64, 64, GRAY, BLACK);
    Texture2D ground_texture = LoadTextureFromImage(gen_ground_image);
    UnloadImage(gen_ground_image);

    // Set texture to repeat mode
    SetTextureWrap(ground_texture, TEXTURE_WRAP_REPEAT);
    SetTextureFilter(ground_texture, TEXTURE_FILTER_BILINEAR);

    // Create a plane ground_mesh
    Mesh ground_mesh = GenMeshPlane(100.0f, 100.0f, 1, 1); // 10x10 units, 1 subdivision
    Model ground_model = LoadModelFromMesh(ground_mesh);

    SetMaterialTexture(&ground_model.materials[0], MATERIAL_MAP_DIFFUSE, ground_texture);
    ground_model.materials[0].shader = shader;
    //----------------GROUND-------------------

    Vector3 bloodRot = {1.0F, 0.0F, 0.0f};
    Vector3 bloodScale = {0.7f, 0.7f, 0.7f};

    Texture2D crosshair = LoadTexture("resources/textures/crosshair.png");
    Texture2D particle_tex = LoadTexture("resources/textures/particle.png");

    BoundingBox dingusHitBox = {0};
    BoundingBox ammoBoxHitBox = {0};
    BoundingBox boxHitBox = {0};
    BoundingBox shotgunHitBox = {0};
    BoundingBox PlayerHitBox = {0};
    BoundingBox NpcHitBox = {0};

    Vector3 shotgunOffset = {weaponCamera.position.x + 0.2f, weaponCamera.position.y - 0.3f, weaponCamera.position.z - 0.4f};

    int cooldown = 120;
    int ammo = 0;
    float ammoWeight = 0.3f;
    float shotgunWeight = 3.5f;
    float currentWeight = 0.0;
    float maxWeight = 45.0f;
    bool pickUpDingus = false;
    bool pickUpBox = false;
    bool equipShotgun = false;
    bool haveShotgun = false;
    bool debug = true;
    bool inventory_show = false;
    bool haveToReload = true;
    bool ammoInChamber = false;
    bool noclip = false;
    bool shader_active = true;
    float bobbingTime = 0.0f;
    float bobbingSpeed = 5.0f;
    float bobbingAmount = 0.1f;
    float restY = 1.1f;
    float restZ = 10.1f;
    float recoilOffset = 1.0f;
    char commandPrompt[256] = {0};
    float cam_rot_x = 0.0f;
    float cam_rot_y = 0.0f;
    float moveSpeed = 0.2f;
    float velY = 0.0f;
    const float gravity = -9.81f; // Gravity constant
    const float cam_sensitivity = 0.003f;
    int fontSize = 10;
    float carryDistance = 2.0f;
    struct nk_context *ctx = InitNuklear(fontSize);

    InitAudioDevice();
    Sound fire = LoadSound("resources/sounds/shot.mp3");
    Sound reload = LoadSound("resources/sounds/reload.mp3");
    Sound walking = LoadSound("resources/sounds/walking.mp3");
    Sound startingVoice = LoadSound("resources/sounds/test_voice.wav");
    // ####################################################################################################
    //                                    FUNCTIONS AND MAIN GAME LOOP
    // ####################################################################################################

    SetTargetFPS(60);
    DisableCursor(); // Lock mouse at start
    PlaySound(startingVoice);

    // printf(TextFormat("%d/%d", crosshair.width/2, crosshair.height/2));
    while (!WindowShouldClose())
    {
        if (!inventory_show)
        {
            zoomControl(&camera, maxFOV, &shotgunOffset);
        }

        // Inventory toggle
        if (IsKeyPressed(KEY_TAB))
        {
            inventory_show = !inventory_show;
            if (inventory_show)
            {
                EnableCursor();
            }
            else
            {
                DisableCursor();
                SetMousePosition(screenWidth / 2, screenHeight / 2); // recenter on close
            }
        }

        // VIEW MODEL
        UpdateCamera(&weaponCamera, CAMERA_CUSTOM);

        // CAMERA CUSTOM AND MOVEMENT
        UpdateCamera(&camera, CAMERA_CUSTOM);

        // Camera rotation and mouse lock
        if (!inventory_show)
        {
            Vector2 mouseDelta = GetMouseDelta();
            cam_rot_x -= mouseDelta.x * cam_sensitivity; // yaw
            cam_rot_y -= mouseDelta.y * cam_sensitivity; // pitch

            // Clamp pitch
            if (cam_rot_y > PI / 2.0f - 0.01f)
                cam_rot_y = PI / 2.0f - 0.01f;
            if (cam_rot_y < -PI / 2.0f + 0.01f)
                cam_rot_y = -PI / 2.0f + 0.01f;

            SetMousePosition(screenWidth / 2, screenHeight / 2);
        }

        camera.position = PlayerPos;

        if (!noclip)
        {
            // Player Gravity and movement
            if (PlayerPos.y > groundLevel)
            {
                velY += gravity * GetFrameTime(); // Apply gravity
            }
            else
            {
                velY = 0.0f;               // Reset velocity when on the ground
                PlayerPos.y = groundLevel; // Ensure player stays above ground
            }
            if (IsKeyDown(KEY_SPACE) && PlayerPos.y <= groundLevel)
            {
                velY = 5.0f; // Jump
            }
            PlayerPos.y += velY * GetFrameTime();
        }

        // Calculate forward and right vectors
        Vector3 cam_forward = {cosf(cam_rot_y) * sinf(cam_rot_x), sinf(cam_rot_y), cosf(cam_rot_y) * cosf(cam_rot_x)};
        Vector3 cam_right = {sinf(cam_rot_x - PI / 2.0f), 0.0f, cosf(cam_rot_x - PI / 2.0f)};

        // flatten cam_forward for movement when noclip is off
        Vector3 move_forward = cam_forward;
        if (!noclip)
        {
            move_forward.y = 0;
            move_forward = Vector3Normalize(move_forward);
        }

        camera.target = Vector3Add(PlayerPos, cam_forward);

        if (!inventory_show)
        {
            bool isMoving = false;
            if (IsKeyDown(KEY_W))
            {
                PlayerPos = Vector3Add(PlayerPos, Vector3Scale(move_forward, moveSpeed));
                isMoving = true;
            }
            if (IsKeyDown(KEY_S))
            {
                PlayerPos = Vector3Subtract(PlayerPos, Vector3Scale(move_forward, moveSpeed));
                isMoving = true;
            }
            if (IsKeyDown(KEY_D))
            {
                PlayerPos = Vector3Add(PlayerPos, Vector3Scale(cam_right, moveSpeed));
                isMoving = true;
            }
            if (IsKeyDown(KEY_A))
            {
                PlayerPos = Vector3Subtract(PlayerPos, Vector3Scale(cam_right, moveSpeed));
                isMoving = true;
            }

            if (isMoving)
            {
                bobbingTime += GetFrameTime() * bobbingSpeed;
                shotgunOffset.y = restY + sinf(bobbingTime) * bobbingAmount;
            }
            else
            {
                shotgunOffset.y += (restY - shotgunOffset.y) * 0.1f;
            }
        }

        // Updating Player HitBox
        PlayerHitboxMin.x = PlayerPos.x - 0.5f;
        PlayerHitboxMin.y = PlayerPos.y - groundLevel;
        PlayerHitboxMin.z = PlayerPos.z - 0.5f;

        PlayerHitboxMax.x = PlayerPos.x + 0.5f;
        PlayerHitboxMax.y = PlayerPos.y + 0.6f;
        PlayerHitboxMax.z = PlayerPos.z + 0.5f;

        BoundingBox PlayerHitBox = {PlayerHitboxMin, PlayerHitboxMax};

        if (inventory_show)
        {
            UpdateNuklear(ctx);
            CustomNuklearStyle(ctx);
            if (nk_begin(ctx, "inventory", nk_rect(100, 100, 450, 450), NK_WINDOW_CLOSABLE | NK_WINDOW_BORDER | NK_WINDOW_MOVABLE))
            {
                nk_layout_row_static(ctx, 200, 190, 1);

                if (nk_group_begin(ctx, "column1", NK_WINDOW_BORDER))
                {
                    nk_layout_row_dynamic(ctx, 15, 1);
                    if (haveShotgun)
                    {
                        nk_layout_row_dynamic(ctx, 30, 1);
                        if (nk_button_label(ctx, "Shotgun"))
                        {
                            equipShotgun = !equipShotgun;
                            shotgunOffset.y = 0.2f; // Adjust shotgun position when equiped
                        }
                        nk_label_wrap(ctx, TextFormat("Ammo: %d", ammo));
                        nk_label_wrap(ctx, "Command Line");
                        nk_edit_string_zero_terminated(ctx, NK_EDIT_FIELD, commandPrompt, sizeof(commandPrompt) - 1, nk_filter_default);
                        if (nk_button_label(ctx, "Done"))
                        {
                            if (strcmp(commandPrompt, "debug") == 0)
                            {
                                debug = !debug;
                            }
                            else if (strcmp(commandPrompt, "noclip") == 0)
                            {
                                noclip = !noclip;
                            }
                            else if (strcmp(commandPrompt, "shader") == 0)
                            {
                                shader_active = !shader_active;
                            }
                        }
                    }
                    nk_group_end(ctx);
                }
            }
            nk_end(ctx);
        }

        Vector3 bloodPos = dingusPos;
        getScaledBoundingBox(&dingusHitBox, dingusMesh, dingusPos, dingusScale);
        getScaledBoundingBox(&ammoBoxHitBox, ammoBoxMesh, ammoBoxPos, ammoBoxScale);
        getScaledBoundingBox(&boxHitBox, boxMesh, boxPos, boxScale);
        getScaledBoundingBox(&shotgunHitBox, shotgunMesh, shotgunPos, shotgunScale);
        getScaledBoundingBox(&shotgunHitBox, shotgunMesh, shotgunPos, shotgunScale);
        getScaledBoundingBox(&NpcHitBox, npcMesh, npcPos, npcScale);

        float cameraPos[3] = {camera.position.x, camera.position.y, camera.position.z};
        SetShaderValue(shader, shader.locs[SHADER_LOC_VECTOR_VIEW], cameraPos, SHADER_UNIFORM_VEC3);
        if (!inventory_show)
        {
            if (IsKeyPressed(KEY_L))
                lights[0].enabled = !lights[0].enabled;
            // if (IsKeyPressed(KEY_L)) lights[1].enabled = !lights[1].enabled;
            // if (IsKeyPressed(KEY_L)) lights[2].enabled = !lights[2].enabled;
            // if (IsKeyPressed(KEY_L)) lights[3].enabled = !lights[3].enabled;
        }

        Ray ray = GetMouseRay(GetMousePosition(), camera);
        RayCollision ammoCollision = GetRayCollisionBox(ray, ammoBoxHitBox);
        RayCollision shotgunCollision = GetRayCollisionBox(ray, shotgunHitBox);
        RayCollision collision = GetRayCollisionBox(ray, dingusHitBox);

        if (equipShotgun && haveShotgun && !inventory_show)
        {
            if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT) && cooldown >= 120 && ammoInChamber && !haveToReload)
            {
                cooldown = 0;
                PlaySound(fire);
                if (collision.hit)
                {
                    dingusHealth--;
                }
                haveToReload = true;
                shotgunOffset.z += recoilOffset; // recoil effect
                ammoInChamber = false;
            }
            else
            {
                shotgunOffset.z += (restZ - shotgunOffset.z) * 0.1f;
            }
        }
        if (equipShotgun && haveShotgun && haveToReload && ammo >= 1 && IsKeyPressed(KEY_R) && !ammoInChamber)
        {
            ammo--;
            ammoInChamber = true;
            currentWeight -= ammoWeight;
            haveToReload = false;
            PlaySound(reload);
        }

        if (cooldown <= 119)
        {
            cooldown++;
        }
        else
        {
            dingusHealth = 1;
        }

        if (IsMouseButtonPressed(MOUSE_BUTTON_RIGHT))
        {
            if (GetRayCollisionBox(ray, dingusHitBox).hit)
                pickUpDingus = !pickUpDingus;
            else if (GetRayCollisionBox(ray, boxHitBox).hit)
                pickUpBox = !pickUpBox;
        }

        if (pickUpDingus)
        {
            dingusPos = Vector3Add(camera.position, Vector3Scale(cam_forward, carryDistance));
        }
        if (pickUpBox)
        {
            boxPos = Vector3Add(camera.position, Vector3Scale(cam_forward, carryDistance));
        }

        if (IsKeyDown(KEY_W) || IsKeyDown(KEY_S) || IsKeyDown(KEY_A) || IsKeyDown(KEY_D))
        {
            if (!IsSoundPlaying(walking))
                PlaySound(walking);
        }
        else
        {
            StopSound(walking);
        }
        if (!inventory_show)
        {
            if (IsKeyPressed(KEY_F3))
                debug = !debug;
        }

        for (int i = 0; i < MAX_LIGHTS; i++)
            UpdateLightValues(shader, lights[i]);

        BeginTextureMode(shader_tex);
        ClearBackground(BLACK);
        BeginBlendMode(BLEND_ALPHA);
        BeginMode3D(camera);
        BeginShaderMode(shader);
        if (dingusHealth == 1)
        {
            DrawModelEx(dingusMesh, dingusPos, dingusRot, 0.0f, dingusScale, WHITE);
        }
        else
        {
            DrawModelEx(bloodMesh, bloodPos, bloodRot, 0.0f, bloodScale, WHITE);
        }
        SpawnBillboardEntity(camera, (Vector3){-20.0f, 1.0f, -20.0f}, particle_tex, 2.0f, BLACK);
        DrawModelEx(tableMesh, tablePos, tableRot, 0.0f, tableScale, WHITE);
        DrawModelEx(pcMesh, pcPos, pcRot, 0.0f, pcScale, WHITE);
        DrawModelEx(carMesh, carPos, carRot, 0.0f, carScale, WHITE);
        DrawModelEx(shotgunMesh, shotgunPos, shotgunRot, 0.0f, shotgunScale, WHITE);
        DrawModelEx(ammoBoxMesh, ammoBoxPos, ammoBoxRot, 90.0f, ammoBoxScale, WHITE);
        DrawModelEx(boxMesh, boxPos, boxRot, 0.0f, boxScale, WHITE);
        DrawModelEx(npcMesh, npcPos, npcRot, 0.0f, npcScale, WHITE);

        if (debug)
        {
            DrawBoundingBox(boxHitBox, RED);
            DrawBoundingBox(ammoBoxHitBox, RED);
            DrawBoundingBox(dingusHitBox, RED);
            DrawBoundingBox(shotgunHitBox, RED);
            DrawBoundingBox(PlayerHitBox, RED);
            DrawBoundingBox(NpcHitBox, RED);
        }

        DrawModel(ground_model, (Vector3){0.0f, 0.0f, 0.0f}, 1.0f, WHITE);
        CreateParticle(camera, (Vector3){20.0f, 1.0f, 20.0f}, shader, 60.0f, YELLOW, ORANGE, GRAY);
        EndShaderMode();

        if (lights[0].enabled)
        {
            DrawSphereEx(lights[0].position, 0.2f, 8, 8, lights[0].color);
            // DrawSphereEx(lights[1].position, 0.2f, 8, 8, lights[1].color);
            // DrawSphereEx(lights[2].position, 0.2f, 8, 8, lights[2].color);
            // DrawSphereEx(lights[3].position, 0.2f, 8, 8, lights[3].color);
        }
        else
        {
            DrawSphereWires(lights[0].position, 0.2f, 8, 8, ColorAlpha(lights[0].color, 0.3f));
            // DrawSphereWires(lights[1].position, 0.2f, 8, 8, ColorAlpha(lights[1].color, 0.3f));
            // DrawSphereWires(lights[2].position, 0.2f, 8, 8, ColorAlpha(lights[2].color, 0.3f));
            // DrawSphereWires(lights[3].position, 0.2f, 8, 8, ColorAlpha(lights[3].color, 0.3f));
        }

        // SpawnBillboardEntity(camera, (Vector3){-20.0f, 1.0f, -20.0f}, particle_tex, 2.0f, BLACK);
        EndMode3D();
        EndBlendMode();

        BeginMode3D(weaponCamera);
        if (haveShotgun && equipShotgun)
        {
            DrawModelEx(shotgunMesh, shotgunOffset, shotgunRot, 10.0f, shotgunScale, WHITE);
        }
        EndMode3D();

        EndTextureMode();

        BeginDrawing();

        BeginShaderMode(shader_pix);
        DrawTextureRec(shader_tex.texture,
                       (Rectangle){0, 0, (float)shader_tex.texture.width, -(float)shader_tex.texture.height},
                       (Vector2){0, 0}, WHITE);
        EndShaderMode();

        // DrawTexture(crosshair, screenWidth / 2 + crosshair.width, screenHeight / 2 + crosshair.height, WHITE);
        if (debug)
        {
            DrawRectangle(0, 0, 600, 150, DEBUG_BACKGROUND);
            DrawFpsEx(font, (Vector2){10, 10}, 12.0f, 1.0f);
            DrawTextEx(font, TextFormat("Cam pos: [X: %.2f Y: %.2f Z: %.2f]", camera.position.x, camera.position.y, camera.position.z), (Vector2){10, 30}, 12, 1, WHITE);
            DrawTextEx(font, TextFormat("Cam target: [X: %.2f Y: %.2f Z: %.2f]", camera.target.x, camera.target.y, camera.target.z), (Vector2){10, 45}, 12, 1, WHITE);
            DrawTextEx(font, TextFormat("Cooldown: %d/120", cooldown), (Vector2){10, 60}, 12, 1, WHITE);
            DrawTextEx(font, TextFormat("Ammo: %d", ammo), (Vector2){10, 75}, 12, 1, WHITE);
            DrawTextEx(font, TextFormat("Current Weight: %.2f/45.0", currentWeight), (Vector2){10, 90}, 12, 1, WHITE);
            DrawTextEx(font, TextFormat("AmmoInChamber: %d", ammoInChamber), (Vector2){10, 105}, 12, 1, WHITE);
            DrawTextEx(font, TextFormat("HaveToReload: %d", haveToReload), (Vector2){10, 120}, 12, 1, WHITE);
            DrawTextEx(font, TextFormat("Camera FOV: %.1ff", camera.fovy), (Vector2){10, 135}, 12, 1, WHITE);
            DrawTimer(font);
            DrawTextLetterByLetter(font, "Hello, This Is A Test Game With Some Test Features.", 100, 200, 20, 2.0f, WHITE, 0.05f);
        }
        DrawRectangle(0, screenHeight - 145, 300, 145, DEBUG_BACKGROUND);
        DrawTextEx(font, "Controls: \n\nMove:   [Z][Q][S][D]\nPickup: [RMB]\nUse:    [E]\nReload: [R]\nZoom/Dezoom: [X]/[C]\nLights: [L]\nDebug:  [F3]", (Vector2){10, screenHeight - 135}, 12, 1, WHITE);
        DrawRectangle((screenWidth / 2) - 1, (screenHeight / 2) - 1, 5, 5, RED);
        if (inventory_show)
        {
            DrawRectangle(0, 0, screenWidth, screenHeight, DEBUG_BACKGROUND);
        }

        if (ammoCollision.hit)
        {
            DrawTextEx(font, "Press RMB to pick up", (Vector2){screenHeight - 20, 20}, 12, 1, WHITE);
            if (IsMouseButtonPressed(MOUSE_BUTTON_RIGHT) && (currentWeight + ammoWeight <= maxWeight))
            {
                ammo++;
                currentWeight += ammoWeight;
            }
            if (currentWeight + ammoWeight >= maxWeight)
            {
                DrawTextEx(font, "Not Enough Space", (Vector2){screenHeight - 20, 35}, 12, 1, RED);
            }
        }

        if (GetRayCollisionBox(ray, boxHitBox).hit)
        {
            DrawTextEx(font, "Press E to use", (Vector2){screenHeight - 20, 20}, 12, 1, WHITE);
            if (IsKeyPressed(KEY_E))
            {
                boxPos.y = -1000.0f;
            }
        }

        if (shotgunCollision.hit)
        {
            DrawTextEx(font, "Press RMB to pick up", (Vector2){screenHeight - 20, 20}, 12, 1, WHITE);
            if (IsMouseButtonPressed(MOUSE_BUTTON_RIGHT) && (currentWeight + shotgunWeight <= maxWeight))
            {
                equipShotgun = true;
                haveShotgun = true;
                shotgunOffset.y = 0.2f; // Adjust shotgun position when picked up
                shotgunPos.y = -1000.0f;
                currentWeight += shotgunWeight;
            }
            if (currentWeight + shotgunWeight >= maxWeight)
            {
                DrawTextEx(font, "Not Enough Space", (Vector2){screenHeight - 20, 35}, 12, 1, RED);
            }
        }
        else if (haveShotgun && equipShotgun && haveToReload && ammo >= 1)
        {
            DrawTextEx(font, "R to Reload", (Vector2){screenWidth / 2 + 10.0f, screenHeight / 2 + 0.5f}, 12, 1, RED);
        }

        DrawNuklear(ctx);
        EndDrawing();
    }
    UnloadSound(fire);
    UnloadSound(walking);
    UnloadSound(reload);

    UnloadModel(dingusMesh);
    UnloadModel(tableMesh);
    UnloadModel(pcMesh);
    UnloadModel(carMesh);
    UnloadModel(shotgunMesh);
    UnloadModel(ammoBoxMesh);
    UnloadModel(boxMesh);
    UnloadModel(bloodMesh);
    UnloadModel(npcMesh);
    UnloadModel(ground_model);

    UnloadTexture(dingus_tex);
    UnloadTexture(table_tex);
    UnloadTexture(pc_tex);
    UnloadTexture(car_tex);
    UnloadTexture(shotgun_tex);
    UnloadTexture(ammoBox_tex);
    UnloadTexture(box_tex);
    UnloadTexture(blood_tex);
    UnloadTexture(crosshair);
    UnloadTexture(particle_tex);
    UnloadTexture(npc_tex);
    UnloadTexture(ground_texture);
    UnloadFont(font);
    UnloadImage(icon);

    UnloadShader(shader);
    UnloadShader(shader_pix);
    UnloadRenderTexture(shader_tex);
    CloseAudioDevice();
    CloseWindow();
    return 0;
}
