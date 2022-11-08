#include "chickendodge/pch/precomp.h"

#include "chickendodge/components.h"

constexpr auto TeamFile = "equipe.txt";
constexpr auto TeamDefaultString = "Coéquipiers";
constexpr auto LaunchScene = "scenes/play.json";
constexpr auto DefaultName = "Anonyme";

namespace ChickenDodge
{
  struct Config
  {
    static Config& Instance()
    {
      static Config instance;
      return instance;
    }

    static void SetName(std::string_view val) { Instance().alias = val; }

    static void SetOtherName(std::string_view val) { Instance().alias2 = val; }

    std::string alias{DefaultName};
    std::string alias2{DefaultName};
  };

  class Game : public SimpleGE::Game
  {
  public:
    Game()
    {
      PlayerComponent::SetNames({Config::Instance().alias, Config::Instance().alias2});
    }

  protected:
    [[nodiscard]] std::string_view GetLaunchScene() const override
    {
      return LaunchScene;
    }

    void SetupSystem() override
    {
      AddSystem<SimpleGE::PhysicSystem>();
      AddSystem<SimpleGE::LogicSystem>();
      AddSystem<SimpleGE::DisplaySystem>();
    }

    void RegisterComponents() const override { RegisterGameComponents(); }
  };

  static void ProcessCommandLine(gsl::span<char*> args)
  {
    SimpleGE::CommandLine::RegisterOption("-U", Config::SetName);
    SimpleGE::CommandLine::RegisterOption("-O", Config::SetOtherName);
    SimpleGE::CommandLine::Parse(args);
  }
} // namespace ChickenDodge

[[nodiscard]] static bool ValidateTeam()
{
  auto teamFile = SimpleGE::Resources::Get<SimpleGE::TextAsset>(TeamFile);
  if (teamFile == nullptr)
  {
    fmt::print(stderr, "Échec d'ouverture de {}\n", TeamFile);
    return false;
  }

  if ((std::string_view) *teamFile == TeamDefaultString)
  {
    fmt::print(stderr, "N'oubliez pas d'inscrire les membres de votre équipe dans le fichier data/equipe.txt!\n");
    return false;
  }

  return true;
}

int main(int argc, char* argv[])
{
  if (!ValidateTeam())
  {
    return -1;
  }

  ChickenDodge::ProcessCommandLine({argv, (size_t) argc});

  ChickenDodge::Game game;
  auto runResult = game.Run();
  return runResult == SimpleGE::Game::RunResult::Success ? 0 : 1;
}