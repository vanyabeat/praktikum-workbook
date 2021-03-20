#pragma once

#include <algorithm>
#include <deque>
#include <utility>
#include <vector>

#include "geo.h"
#include "json.h"
#include "svg.h"
#include "transport_catalogue.h"

inline const double EPSILON = 1e-6;

namespace Handbook
{
	namespace Renderer
	{
		using BusesByName = std::map<std::string_view, Handbook::Data::BusPtr>;
		struct RenderSettings
		{
			double width = 0.0;
			double height = 0.0;
			double padding = 0.0;
			double line_width = 0.0;
			double stop_radius = 0.0;
			int bus_label_font_size = 0;
			svg::Point bus_label_offset{0, 0};
			int stop_label_font_size = 0;
			svg::Point stop_label_offset{0, 0};
			svg::Color underlayer_color{svg::NoneColor};
			double underlayer_width = 0.0;
			std::vector<svg::Color> color_palette;
		};

		bool IsZero(double value);

		class SphereProjector
		{
		  public:
			template <typename PointInputIt>
			SphereProjector(PointInputIt points_begin, PointInputIt points_end, double max_width, double max_height,
							double padding)
				: padding_(padding)
			{
				if (points_begin == points_end)
				{
					return;
				}

				const auto [left_it, right_it] =
					std::minmax_element(points_begin, points_end, [](auto lhs, auto rhs) { return lhs.lng < rhs.lng; });
				min_lon_ = left_it->lng;
				const double max_lon = right_it->lng;

				const auto [bottom_it, top_it] =
					std::minmax_element(points_begin, points_end, [](auto lhs, auto rhs) { return lhs.lat < rhs.lat; });
				const double min_lat = bottom_it->lat;
				max_lat_ = top_it->lat;

				std::optional<double> width_zoom;
				if (!IsZero(max_lon - min_lon_))
				{
					width_zoom = (max_width - 2 * padding) / (max_lon - min_lon_);
				}

				std::optional<double> height_zoom;
				if (!IsZero(max_lat_ - min_lat))
				{
					height_zoom = (max_height - 2 * padding) / (max_lat_ - min_lat);
				}

				if (width_zoom && height_zoom)
				{
					zoom_coeff_ = std::min(*width_zoom, *height_zoom);
				}
				else if (width_zoom)
				{
					zoom_coeff_ = *width_zoom;
				}
				else if (height_zoom)
				{
					zoom_coeff_ = *height_zoom;
				}
			}

			svg::Point operator()(Handbook::Utilities::Coordinates coords) const
			{
				return {(coords.lng - min_lon_) * zoom_coeff_ + padding_,
						(max_lat_ - coords.lat) * zoom_coeff_ + padding_};
			}

		  private:
			double padding_ = 0;
			double min_lon_ = 0;
			double max_lat_ = 0;
			double zoom_coeff_ = 0;
		};

		class Map
		{
		  public:
			Map() = default;

			explicit Map(RenderSettings props) : render_props_(std::move(props))
			{
			}

			void SetRenderProps(RenderSettings props)
			{
				render_props_ = std::move(props);
			}

			void Render(const BusesByName& buses_by_name, std::ostream& out = std::cout) const;

		  private:
			RenderSettings render_props_;

			void DrawLineOfRoad(svg::Document& document, const SphereProjector& projector,
								const BusesByName& buses) const;

			void DrawBusTitles(svg::Document& document, const SphereProjector& projector,
							   const BusesByName& buses) const;

			void DrawBusTitle(svg::Document& document, std::string_view name, svg::Point pos,
							  const svg::Color& color) const;

			void DrawStops(svg::Document& document, const SphereProjector& projector,
						   const std::map<std::string_view, Handbook::Data::StopPtr>& stops) const;

			void DrawStopTitles(svg::Document& document, const SphereProjector& projector,
								const std::map<std::string_view, Handbook::Data::StopPtr>& stops) const;
		};
	} // namespace Renderer
} // namespace Handbook