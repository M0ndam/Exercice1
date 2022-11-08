#include <simplege/simplege.h>

#include "simplege/components/rawsprite.h"
#include "texture.h"

using json = nlohmann::json;

namespace SimpleGE
{
  static void from_json(const json& j, RawSpriteComponent::Description& desc)
  {
    from_json(j, static_cast<TextureComponent::Description&>(desc));

    if (j.contains("width"))
    {
      j.at("width").get_to(desc.width);
    }
    if (j.contains("height"))
    {
      j.at("height").get_to(desc.height);
    }
    if (j.contains("scale"))
    {
      j.at("scale").get_to(desc.scale);
    }
  }

  Component::SetupResult RawSpriteComponent::Setup(const nlohmann::json& descr)
  {
    return Setup(descr.get<Description>());
  }

  Component::SetupResult RawSpriteComponent::Setup(const Description& descr)
  {
    // On charge l'image et les shaders
    auto status = TextureComponent::Setup(descr);
    Ensures(status.IsDone());

    // On crée ici un tableau de 4 vertices permettant de représenter
    // le rectangle à afficher.
    vertexBuffer = Graphic::CreateVertexBuffer<Vertex, Graphic::BufferUsage::Static>(vertices.size());

    // On crée ici un tableau de 6 indices, soit 2 triangles, pour
    // représenter quels vertices participent à chaque triangle:
    // ```
    // 0    1
    // +----+
    // |\   |
    // | \  |
    // |  \ |
    // |   \|
    // +----+
    // 3    2
    // ```
    indexBuffer = Graphic::CreateIndexBuffer<std::uint16_t, Graphic::BufferUsage::Static>(Indices.size());

    // Et on initialise le contenu des vertices
    UpdateComponents(descr, *GetTexture());

    return {this, {}};
  }

  void RawSpriteComponent::Display(const Timing& timing)
  {
    Bind(vertexBuffer, indexBuffer);
    Graphic::GL::DrawElements(Indices);
    Unbind();
  }

  // Cette méthode met à jour le contenu de chaque vertex.
  void RawSpriteComponent::UpdateComponents(const Description& descr, Image& texture)
  {
    gsl::not_null<PositionComponent*> posComp = Owner().GetComponent<PositionComponent>();
    const auto position = posComp->WorldPosition();
    float width = descr.width ? descr.width : float(texture.Width());
    float height = descr.height ? descr.height : float(texture.Height());
    width *= descr.scale;
    height *= descr.scale;

    const auto z = position[2];
    const auto xMin = position[0] - width / 2.F;
    const auto xMax = xMin + width;
    const auto yMax = position[1] - height / 2.F;
    const auto yMin = yMax - height;

    std::array<Vertex, 4> v{{
        {xMin, yMin, z, 0.F, 0.F},
        {xMax, yMin, z, 1.F, 0.F},
        {xMax, yMax, z, 1.F, 1.F},
        {xMin, yMax, z, 0.F, 1.F},
    }};

    std::copy(v.begin(), v.end(), vertices.begin());
    vertexBuffer->Update(gsl::span<const Vertex>(vertices), 0);
    indexBuffer->Update(gsl::span<const std::uint16_t>(Indices), 0);
  }
} // namespace SimpleGE