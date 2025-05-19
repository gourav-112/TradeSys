#include "imgui.h"
#include "imgui_impl_sdl2.h"
#include "imgui_impl_opengl3.h"
  
#include <SDL.h>
#include <SDL_opengl.h>
#include <stdio.h>
#include <string>
#include <tradeSim.h>
#include <datafeed.h>

// Input parameter variables
std::string exchange = "OKX";
std::string spotAsset = "BTC-USDT";
std::string orderType = "Market";
float quantity = 100.0f;
float volatility = 0.05f;
int feeTier = 1;

// Output parameters (dummy for now)
float expectedSlippage = 0.0f;
float expectedFees = 0.0f;
float expectedMarketImpact = 0.0f;
float netCost = 0.0f;
float makerTakerRatio = 0.0f;
float internalLatency = 0.0f;

// Flags for actions
bool startTrade = false;
bool hasOutput = false;

void simulateTrade(
    float quantity,
    float volatility,
    int feeTier,
    float& expectedSlippage,
    float& expectedFees,
    float& expectedMarketImpact,
    float& netCost,
    float& makerTakerRatio,
    float& internalLatency
) {
    // -- Sample real logic (replace with actual model later) --

    float basePrice = 27000.0f; // Simulate base price
    float simulatedPrice = basePrice * (1.0f + volatility);
    expectedSlippage = simulatedPrice - basePrice;

    float feeRate = 0.001f * feeTier; // Simulated fee tier impact
    expectedFees = quantity * feeRate;

    expectedMarketImpact = 0.0005f * quantity;
    netCost = quantity + expectedSlippage + expectedFees + expectedMarketImpact;

    makerTakerRatio = 0.25f + 0.75f * volatility;
    internalLatency = 300 + (1000 * volatility); // in microseconds
}


// Main function
// This is where the program starts executing
int main(int, char**)
{
    // Setup SDL
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER | SDL_INIT_GAMECONTROLLER) != 0)
    {
        printf("Error: %s\n", SDL_GetError());
        return -1;
    }

    // Setup window with OpenGL context
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, 0);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
    SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
    SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 8);

    SDL_Window* window = SDL_CreateWindow("TradeSim UI", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 1280, 720, SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE);
    SDL_GLContext gl_context = SDL_GL_CreateContext(window);
    SDL_GL_MakeCurrent(window, gl_context);
    SDL_GL_SetSwapInterval(1); // Enable vsync

    // Setup Dear ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;

    // Setup Dear ImGui style
    ImGui::StyleColorsDark();

    // Setup Platform/Renderer bindings
    ImGui_ImplSDL2_InitForOpenGL(window, gl_context);
    ImGui_ImplOpenGL3_Init("#version 330");

    bool show_demo_window = false;

    // Main loop
    bool done = false;
    while (!done)
    {
        // Poll and handle events
        SDL_Event event;
        while (SDL_PollEvent(&event))
        {
            ImGui_ImplSDL2_ProcessEvent(&event);
            if (event.type == SDL_QUIT)
                done = true;
            if (event.type == SDL_WINDOWEVENT && event.window.event == SDL_WINDOWEVENT_CLOSE && event.window.windowID == SDL_GetWindowID(window))
                done = true;
        }

        // Start the Dear ImGui frame
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplSDL2_NewFrame();
        ImGui::NewFrame();

        if (startTrade)
        {
            // Simulated logic (you'll later replace this with real computation)
            expectedSlippage = quantity * volatility * 0.001f;
            expectedFees = quantity * 0.001f * feeTier;
            expectedMarketImpact = quantity * volatility * 0.0005f;
            makerTakerRatio = (orderType == "Limit") ? 0.7f : 0.3f;
            internalLatency = 500.0f + (float)(rand() % 100);
            netCost = expectedSlippage + expectedFees + expectedMarketImpact;

            hasOutput = true;
            startTrade = false;  // reset button flag
        }


        // Two panel UI layout
        ImGui::Begin("TradeSim");

        ImGui::Columns(2, NULL, true);

        // Left panel: Inputs
        ImGui::Text("Input Parameters");
        ImGui::Separator();

        static char exchangeBuf[64] = "OKX";
        ImGui::InputText("Exchange", exchangeBuf, IM_ARRAYSIZE(exchangeBuf));

        static char spotAssetBuf[64] = "BTC-USDT";
        ImGui::InputText("Spot Asset", spotAssetBuf, IM_ARRAYSIZE(spotAssetBuf));

        static char orderTypeBuf[64] = "Market";
        ImGui::InputText("Order Type", orderTypeBuf, IM_ARRAYSIZE(orderTypeBuf));

        ImGui::InputFloat("Quantity (USD)", &quantity, 1.0f, 10.0f, "%.2f");

        ImGui::InputFloat("Volatility", &volatility, 0.01f, 0.1f, "%.4f");

        ImGui::InputInt("Fee Tier", &feeTier);


        static TradeResult lastTradeResult{};
        static bool tradeStarted = false;

        // Input fields (exchangeBuf, spotAssetBuf, orderTypeBuf, quantity, feeTier, etc.) as before

        if (ImGui::Button("Start Trade"))
        {
            // Build or get current order book snapshot (replace this with your real live orderbook data)
            OrderBookSnapshot snapshot = getCurrentOrderBookSnapshot();

            // Map orderTypeBuf to Side enum (here assuming Market order and side Buy for example)
            Side side = Side::Buy;  // Or set based on user input

            // Call simulateMarketOrder
            lastTradeResult = simulateMarketOrder(
                snapshot,
                side,
                quantity,
                feeTier,
                std::chrono::steady_clock::now()
            );

            tradeStarted = true;
        }

        // Output panel
        if (tradeStarted)
        {
            ImGui::Text("Executed Quantity: %.6f", lastTradeResult.executedQuantity);
            ImGui::Text("Average Price: %.6f", lastTradeResult.averagePrice);
            ImGui::Text("Total Cost: %.6f", lastTradeResult.totalCost);
            ImGui::Text("Total Proceeds: %.6f", lastTradeResult.totalProceeds);
            ImGui::Text("Slippage: %.6f%%", lastTradeResult.slippage * 100);
            ImGui::Text("Fees Paid: %.6f", lastTradeResult.feesPaid);
            ImGui::Text("Market Impact: %.6f%%", lastTradeResult.marketImpact * 100);
            ImGui::Text("Maker/Taker Ratio: %.2f", lastTradeResult.makerTakerRatio);
            ImGui::Text("Internal Latency (µs): %.0f", lastTradeResult.internalLatency);
        }


        // Add some spacing before the button
        ImGui::Spacing();

        // Sync input fields to internal variables
        exchange = std::string(exchangeBuf);
        spotAsset = std::string(spotAssetBuf);
        orderType = std::string(orderTypeBuf);

        if (ImGui::Button("Start Trade")) {
            startTrade = true;
        }

        ImGui::NextColumn();

        // Right panel: Outputs
        ImGui::Text("Output Parameters");
        ImGui::Separator();

        if (hasOutput)
        {
            ImGui::Text("Expected Slippage: %.6f", expectedSlippage);
            ImGui::Text("Expected Fees: %.6f", expectedFees);
            ImGui::Text("Expected Market Impact: %.6f", expectedMarketImpact);
            ImGui::Text("Net Cost: %.6f", netCost);
            ImGui::Text("Maker/Taker Proportion: %.6f", makerTakerRatio);
            ImGui::Text("Internal Latency (µs): %.0f", internalLatency);
        }
        else
        {
            ImGui::Text("Click 'Start Trade' to calculate.");
        }


        ImGui::End();

        // Rendering
        ImGui::Render();
        SDL_GL_MakeCurrent(window, gl_context);
        glViewport(0, 0, (int)io.DisplaySize.x, (int)io.DisplaySize.y);
        glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
        SDL_GL_SwapWindow(window);
    }

    // Cleanup
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplSDL2_Shutdown();
    ImGui::DestroyContext();

    SDL_GL_DeleteContext(gl_context);
    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
}
