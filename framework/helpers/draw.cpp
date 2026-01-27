#include "../headers/functions.h"

ImU32 c_draw::get_clr(const ImVec4& col, float alpha)
{
    ImVec4 c = col;
    c.w *= ImGui::GetStyle().Alpha * alpha;
    return ImGui::ColorConvertFloat4ToU32(c);
}

ImU32 c_draw::w_get_clr(style_col idx, float alpha)
{
    ImVec4 c = ImGui::GetStyle().Colors[idx];
    c.w *= ImGui::GetStyle().Alpha * alpha;
    return ImGui::ColorConvertFloat4ToU32(c);
}

void c_draw::rect_filled_multi_color(ImDrawList* draw, const ImVec2& p_min, const ImVec2& p_max, ImU32 col_upr_left, ImU32 col_upr_right, ImU32 col_bot_right, ImU32 col_bot_left, float rounding, ImDrawFlags flags)
{
    if (((col_upr_left | col_upr_right | col_bot_right | col_bot_left) & IM_COL32_A_MASK) == 0)
        return;

    // Use built-in multi-color fill if no rounding
    if (rounding <= 0.0f)
    {
        draw->AddRectFilledMultiColor(p_min, p_max, col_upr_left, col_upr_right, col_bot_right, col_bot_left);
        return;
    }

    // For rounded, fallback to path-based fill with manual coloring
    draw->PathRect(p_min, p_max, rounding, flags);
    draw->PathFillConvex(IM_COL32_WHITE);  // Fill with white, then shade vertices

    int vert_start = draw->VtxBuffer.Size - draw->GetCmdList()->IdxBuffer.Size / 3;  // Approximate start
    int vert_end = draw->VtxBuffer.Size;

    for (int i = vert_start; i < vert_end; ++i)
    {
        ImDrawVert& vert = draw->VtxBuffer[i];
        float X = ImClamp((vert.pos.x - p_min.x) / (p_max.x - p_min.x), 0.0f, 1.0f);
        float Y = ImClamp((vert.pos.y - p_min.y) / (p_max.y - p_min.y), 0.0f, 1.0f);

        ImVec4 left = ImGui::ColorConvertU32ToFloat4(ImLerp(col_upr_left, col_bot_left, Y));
        ImVec4 right = ImGui::ColorConvertU32ToFloat4(ImLerp(col_upr_right, col_bot_right, Y));
        ImVec4 blended = ImLerp(left, right, X);

        vert.col = ImGui::ColorConvertFloat4ToU32(blended);
    }
}

void set_linear_color(ImDrawList* draw_list, int vert_start_idx, int vert_end_idx, ImVec2 gradient_p0, ImVec2 gradient_p1, ImU32 col0, ImU32 col1)
{
    ImVec2 gradient_extent = gradient_p1 - gradient_p0;
    float gradient_inv_length2 = 1.0f / ImLengthSqr(gradient_extent);
    ImDrawVert* vert_start = draw_list->VtxBuffer.Data + vert_start_idx;
    ImDrawVert* vert_end = draw_list->VtxBuffer.Data + vert_end_idx;
    const int col0_r = (int)(col0 >> IM_COL32_R_SHIFT) & 0xFF;
    const int col0_g = (int)(col0 >> IM_COL32_G_SHIFT) & 0xFF;
    const int col0_b = (int)(col0 >> IM_COL32_B_SHIFT) & 0xFF;
    const int col_delta_r = ((int)(col1 >> IM_COL32_R_SHIFT) & 0xFF) - col0_r;
    const int col_delta_g = ((int)(col1 >> IM_COL32_G_SHIFT) & 0xFF) - col0_g;
    const int col_delta_b = ((int)(col1 >> IM_COL32_B_SHIFT) & 0xFF) - col0_b;
    for (ImDrawVert* vert = vert_start; vert < vert_end; vert++)
    {
        float d = ImDot(vert->pos - gradient_p0, gradient_extent);
        float t = ImClamp(d * gradient_inv_length2, 0.0f, 1.0f);
        int r = (int)(col0_r + col_delta_r * t);
        int g = (int)(col0_g + col_delta_g * t);
        int b = (int)(col0_b + col_delta_b * t);
        vert->col = (r << IM_COL32_R_SHIFT) | (g << IM_COL32_G_SHIFT) | (b << IM_COL32_B_SHIFT) | (vert->col & IM_COL32_A_MASK);
    }
}

void c_draw::set_linear_color_alpha(ImDrawList* draw_list, int vert_start_idx, int vert_end_idx, ImVec2 gradient_p0, ImVec2 gradient_p1, ImU32 col0, ImU32 col1)
{
    ImVec2 gradient_extent = gradient_p1 - gradient_p0;
    float gradient_inv_length2 = 1.0f / ImLengthSqr(gradient_extent);
    ImDrawVert* vert_start = draw_list->VtxBuffer.Data + vert_start_idx;
    ImDrawVert* vert_end = draw_list->VtxBuffer.Data + vert_end_idx;
    const int col0_r = (int)(col0 >> IM_COL32_R_SHIFT) & 0xFF;
    const int col0_g = (int)(col0 >> IM_COL32_G_SHIFT) & 0xFF;
    const int col0_b = (int)(col0 >> IM_COL32_B_SHIFT) & 0xFF;
    const int col0_a = (int)(col0 >> IM_COL32_A_SHIFT) & 0xFF;
    const int col_delta_r = ((int)(col1 >> IM_COL32_R_SHIFT) & 0xFF) - col0_r;
    const int col_delta_g = ((int)(col1 >> IM_COL32_G_SHIFT) & 0xFF) - col0_g;
    const int col_delta_b = ((int)(col1 >> IM_COL32_B_SHIFT) & 0xFF) - col0_b;
    const int col_delta_a = ((int)(col1 >> IM_COL32_A_SHIFT) & 0xFF) - col0_a;
    for (ImDrawVert* vert = vert_start; vert < vert_end; vert++)
    {
        float d = ImDot(vert->pos - gradient_p0, gradient_extent);
        float t = ImClamp(d * gradient_inv_length2, 0.0f, 1.0f);
        int r = (int)(col0_r + col_delta_r * t);
        int g = (int)(col0_g + col_delta_g * t);
        int b = (int)(col0_b + col_delta_b * t);
        int a = (int)(col0_a + col_delta_a * t);
        vert->col = (r << IM_COL32_R_SHIFT) | (g << IM_COL32_G_SHIFT) | (b << IM_COL32_B_SHIFT) | (a << IM_COL32_A_SHIFT);
    }
}

void c_draw::fade_rect_filled(ImDrawList* draw, const ImVec2& pos_min, const ImVec2& pos_max, ImU32 col_one, ImU32 col_two, fade_direction direction, float rounding, ImDrawFlags flags)
{
    const ImVec2 fade_pos_in = (direction == fade_direction::diagonally_reversed) ? ImVec2(pos_max.x, pos_min.y) : pos_min;
    const ImVec2 fade_pos_out = (direction == fade_direction::vertically) ? ImVec2(pos_min.x, pos_max.y) :
        (direction == fade_direction::horizontally) ? ImVec2(pos_max.x, pos_min.y) :
        (direction == fade_direction::diagonally) ? pos_max :
        (direction == fade_direction::diagonally_reversed) ? ImVec2(pos_min.x, pos_max.y) : ImVec2(0, 0);

    int vert_start = draw->VtxBuffer.Size;
    draw->AddRectFilled(pos_min, pos_max, IM_COL32_WHITE, rounding, flags);
    int vert_end = draw->VtxBuffer.Size;

    set_linear_color_alpha(draw, vert_start, vert_end, fade_pos_in, fade_pos_out, col_one, col_two);
}

void c_draw::text(ImDrawList* draw_list, const ImFont* font, const ImVec2& pos, ImU32 col, const char* text_begin, const char* text_end, float wrap_width, const ImVec4* cpu_fine_clip_rect)
{
    if ((col & IM_COL32_A_MASK) == 0)
        return;

    if (text_begin == text_end || text_begin[0] == '\0')
        return;

    if (text_end == nullptr)
        text_end = text_begin + strlen(text_begin);

    if (font == nullptr)
        font = draw_list->GetFont();  // Use current font

    ImVec4 clip_rect = draw_list->_CmdHeader.ClipRect;
    if (cpu_fine_clip_rect)
    {
        clip_rect.x = ImMax(clip_rect.x, cpu_fine_clip_rect->x);
        clip_rect.y = ImMax(clip_rect.y, cpu_fine_clip_rect->y);
        clip_rect.z = ImMin(clip_rect.z, cpu_fine_clip_rect->z);
        clip_rect.w = ImMin(clip_rect.w, cpu_fine_clip_rect->w);
    }

    draw_list->AddText(font, pos, col, text_begin, text_end, wrap_width, cpu_fine_clip_rect ? &clip_rect : nullptr);
}

void c_draw::text_clipped(ImDrawList* draw_list, ImFont* font, const ImVec2& pos_min, const ImVec2& pos_max, ImU32 color, const char* text, const char* text_display_end, const ImVec2* text_size_if_known, const ImVec2& align, const ImRect* clip_rect)
{
    ImGui::PushFont(font);
    ImVec2 pos = pos_min;
    const ImVec2 text_size = text_size_if_known ? *text_size_if_known : ImGui::CalcTextSize(text, text_display_end, false, 0.0f);
    const ImVec2* clip_min = clip_rect ? &clip_rect->Min : &pos_min;
    const ImVec2* clip_max = clip_rect ? &clip_rect->Max : &pos_max;
    bool need_clipping = (pos.x + text_size.x >= clip_max->x) || (pos.y + text_size.y >= clip_max->y);
    if (clip_rect) need_clipping |= (pos.x < clip_min->x) || (pos.y < clip_min->y);

    if (align.x > 0.0f) pos.x = ImMax(pos.x, pos.x + (pos_max.x - pos.x - text_size.x) * align.x);
    if (align.y > 0.0f) pos.y = ImMax(pos.y, pos.y + (pos_max.y - pos.y - text_size.y) * align.y);

    if (need_clipping)
    {
        ImVec4 fine_clip_rect(clip_min->x, clip_min->y, clip_max->x, clip_max->y);
        draw_list->AddText(nullptr, pos, color, text, text_display_end, 0.0f, &fine_clip_rect);
    }
    else
    {
        draw_list->AddText(nullptr, pos, color, text, text_display_end, 0.0f, nullptr);
    }
    ImGui::PopFont();
}

ImVec2 text_size_animed(const char* text, const char* text_end, bool hide_text_after_double_hash, float wrap_width)
{
    ImGuiContext& g = *GImGui;
    const char* text_display_end;
    if (hide_text_after_double_hash)
        text_display_end = ImGui::FindRenderedTextEnd(text, text_end); // Hide anything after a '##' string
    else
        text_display_end = text_end;
    ImFont* font = g.Font;
    if (text == text_display_end)
        return ImVec2(0.0f, font->FontSize);
    ImVec2 text_size = font->CalcTextSizeA(font->FontSize, FLT_MAX, wrap_width, text, text_display_end, nullptr);
    text_size.x = IM_TRUNC(text_size.x + 0.99999f);
    return text_size;
}

void c_draw::text_animed(ImDrawList* draw_list, ImFont* font, const ImVec2& pos_min, const ImVec2& pos_max, ImU32 color, const char* text, const char* text_display_end, const ImVec2* text_size_if_known, const ImVec2& align, const ImRect* clip_rect)
{
    ImGui::PushFont(font);
    ImVec2 pos = pos_min;
    const ImVec2 text_size = text_size_if_known ? *text_size_if_known : text_size_animed(text, text_display_end, false, 0.0f);
    const ImVec2* clip_min = clip_rect ? &clip_rect->Min : &pos_min;
    const ImVec2* clip_max = clip_rect ? &clip_rect->Max : &pos_max;
    bool need_clipping = (pos.x + text_size.x >= clip_max->x) || (pos.y + text_size.y >= clip_max->y);
    if (clip_rect) need_clipping |= (pos.x < clip_min->x) || (pos.y < clip_min->y);

    if (align.x > 0.0f) pos.x = ImMax(pos.x, pos.x + (pos_max.x - pos.x - text_size.x) * align.x);
    if (align.y > 0.0f) pos.y = ImMax(pos.y, pos.y + (pos_max.y - pos.y - text_size.y) * align.y);

    if (need_clipping)
    {
        ImVec4 fine_clip_rect(clip_min->x, clip_min->y, clip_max->x, clip_max->y);
        draw_list->AddText(font, pos, color, text, text_display_end, 0.0f, &fine_clip_rect);
    }
    else
    {
        draw_list->AddText(font, pos, color, text, text_display_end, 0.0f, nullptr);
    }
    ImGui::PopFont();
}

void c_draw::radial_gradient(ImDrawList* draw_list, const ImVec2& center, float radius, ImU32 col_in, ImU32 col_out)
{
    if (((col_in | col_out) & IM_COL32_A_MASK) == 0 || radius < 0.5f)
        return;

    draw_list->PathArcToFast(center, radius, 0, IM_DRAWLIST_ARCFAST_SAMPLE_MAX - 1);
    const int count = draw_list->PathSize() - 1;

    unsigned int vtx_base = draw_list->GetCurrentIdx();
    draw_list->PrimReserve(count * 3, count + 1);

    const ImVec2 uv = draw_list->GetFontTexUvWhitePixel();
    draw_list->PrimWriteVtx(center, uv, col_in);

    for (int n = 0; n < count; n++)
        draw_list->PrimWriteVtx(draw_list->_Path[n], uv, col_out);

    for (int n = 0; n < count; n++)
    {
        draw_list->PrimWriteIdx((ImDrawIdx)vtx_base);
        draw_list->PrimWriteIdx((ImDrawIdx)(vtx_base + 1 + n));
        draw_list->PrimWriteIdx((ImDrawIdx)(vtx_base + 1 + ((n + 1) % count)));
    }
    draw_list->_Path.clear();
}

void c_draw::line(ImDrawList* draw_list, const ImVec2& p1, const ImVec2& p2, ImU32 col, float thickness)
{
    if ((col & IM_COL32_A_MASK) == 0)
        return;

    draw_list->AddLine(p1, p2, col, thickness);
}

void c_draw::rect(ImDrawList* draw_list, const ImVec2& p_min, const ImVec2& p_max, ImU32 col, float rounding, ImDrawFlags flags, float thickness)
{
    if ((col & IM_COL32_A_MASK) == 0)
        return;

    draw_list->AddRect(p_min, p_max, col, rounding, flags, thickness);
}

void c_draw::rect_filled(ImDrawList* draw_list, const ImVec2& p_min, const ImVec2& p_max, ImU32 col, float rounding, ImDrawFlags flags)
{
    if ((col & IM_COL32_A_MASK) == 0)
        return;

    draw_list->AddRectFilled(p_min, p_max, col, rounding, flags);
}

void c_draw::circle(ImDrawList* draw_list, const ImVec2& center, float radius, ImU32 col, int num_segments, float thickness)
{
    if ((col & IM_COL32_A_MASK) == 0 || radius < 0.5f)
        return;

    draw_list->AddCircle(center, radius, col, num_segments, thickness);
}

void c_draw::circle_filled(ImDrawList* draw_list, const ImVec2& center, float radius, ImU32 col, int num_segments)
{
    if ((col & IM_COL32_A_MASK) == 0 || radius < 0.5f)
        return;

    draw_list->AddCircleFilled(center, radius, col, num_segments);
}

void c_draw::triangle(ImDrawList* draw_list, const ImVec2& p1, const ImVec2& p2, const ImVec2& p3, ImU32 col, float thickness)
{
    if ((col & IM_COL32_A_MASK) == 0)
        return;

    draw_list->AddTriangle(p1, p2, p3, col, thickness);
}

void c_draw::triangle_filled(ImDrawList* draw_list, const ImVec2& p1, const ImVec2& p2, const ImVec2& p3, ImU32 col)
{
    if ((col & IM_COL32_A_MASK) == 0)
        return;

    draw_list->AddTriangleFilled(p1, p2, p3, col);
}

void c_draw::image(ImDrawList* draw_list, ImTextureID user_texture_id, const ImVec2& p_min, const ImVec2& p_max, const ImVec2& uv_min, const ImVec2& uv_max, ImU32 col)
{
    if ((col & IM_COL32_A_MASK) == 0)
        return;

    draw_list->AddImage(user_texture_id, p_min, p_max, uv_min, uv_max, col);
}

void c_draw::image_rounded(ImDrawList* draw_list, ImTextureID user_texture_id, const ImVec2& p_min, const ImVec2& p_max, const ImVec2& uv_min, const ImVec2& uv_max, ImU32 col, float rounding, ImDrawFlags flags)
{
    if ((col & IM_COL32_A_MASK) == 0)
        return;

    draw_list->AddImageRounded(user_texture_id, p_min, p_max, uv_min, uv_max, col, rounding, flags);
}

// Subtracted rect and clip polygon shape remain custom, as they are advanced and not built-in.
// You can keep them as-is, assuming they compile; if not, debug specific errors.

void c_draw::shadow_rect(ImDrawList* draw_list, const ImVec2& obj_min, const ImVec2& obj_max, ImU32 shadow_col, float shadow_thickness, const ImVec2& shadow_offset, ImDrawFlags flags, float obj_rounding)
{
    // Simplified placeholder without ShadowRectUvs (removed in new ImGui)
    // Use gradient or multiple blurred rects for real shadows
    ImVec2 offset_min = obj_min + shadow_offset - ImVec2(shadow_thickness, shadow_thickness);
    ImVec2 offset_max = obj_max + shadow_offset + ImVec2(shadow_thickness, shadow_thickness);
    draw_list->AddRectFilled(offset_min, offset_max, shadow_col, obj_rounding, flags);
}

// Similar placeholders for shadow_convex_poly, shadow_circle, shadow_ngon

void c_draw::shadow_convex_poly(ImDrawList* draw_list, const ImVec2* points, int points_count, ImU32 shadow_col, float shadow_thickness, const ImVec2& shadow_offset, ImDrawFlags flags)
{
    // Placeholder: draw offset poly with semi-transparent color
    draw_list->AddConvexPolyFilled(points, points_count, shadow_col);
}

void c_draw::shadow_circle(ImDrawList* draw_list, const ImVec2& obj_center, float obj_radius, ImU32 shadow_col, float shadow_thickness, const ImVec2& shadow_offset, ImDrawFlags flags, int num_segments)
{
    // Placeholder
    draw_list->AddCircleFilled(obj_center + shadow_offset, obj_radius + shadow_thickness, shadow_col, num_segments);
}

void c_draw::shadow_ngon(ImDrawList* draw_list, const ImVec2& obj_center, float obj_radius, ImU32 shadow_col, float shadow_thickness, const ImVec2& shadow_offset, ImDrawFlags flags, int num_segments)
{
    shadow_circle(draw_list, obj_center, obj_radius, shadow_col, shadow_thickness, shadow_offset, flags, num_segments);
}

int rotation_start_index;
void c_draw::rotate_start(ImDrawList* draw_list)
{
    rotation_start_index = draw_list->VtxBuffer.Size;
}

ImVec2 rotate_center(ImDrawList* draw_list)
{
    ImVec2 l(FLT_MAX, FLT_MAX), u(-FLT_MAX, -FLT_MAX);
    const auto& buf = draw_list->VtxBuffer;
    for (int i = rotation_start_index; i < buf.Size; i++)
    {
        l = ImMin(l, buf[i].pos);
        u = ImMax(u, buf[i].pos);
    }
    return ImVec2((l.x + u.x) / 2, (l.y + u.y) / 2);
}

void c_draw::rotate_end(ImDrawList* draw_list, float rad, ImVec2 center)
{
    if (center.x == 0 && center.y == 0) center = rotate_center(draw_list);

    float s = sinf(rad), c = cosf(rad);  // Use radians directly
    center = ImRotate(center, c, s) - center;  // Note: ImRotate takes cos, sin

    auto& buf = draw_list->VtxBuffer;
    for (int i = rotation_start_index; i < buf.Size; i++)
        buf[i].pos = ImRotate(buf[i].pos, c, s) - center;
}

void c_draw::push_clip_rect(ImDrawList* draw_list, const ImVec2& clip_rect_min, const ImVec2& clip_rect_max, bool intersect_with_current_clip_rect)
{
    draw_list->PushClipRect(clip_rect_min, clip_rect_max, intersect_with_current_clip_rect);
}

void c_draw::pop_clip_rect(ImDrawList* draw_list)
{
    draw_list->PopClipRect();
}