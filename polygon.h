#pragma once
#include <iostream>
#include <windows.h>
#include <conio.h>
#include <math.h>
#include <vector>

using namespace std;

wchar_t* charToWchar(const char* str)
{
	size_t len = strlen(str) + 1;
	wchar_t* wstr = new wchar_t[len];
	size_t converted = 0;
	mbstowcs_s(&converted, wstr, len, str, _TRUNCATE);
	return wstr;
}

class c_Polygon
{
private:
	POINT calculate_center(vector<POINT> vertices) {
		double sum_x = 0.0;
		double sum_y = 0.0;
		int num_points = vertices.size();

		for (const auto& p : vertices) {
			sum_x += p.x;
			sum_y += p.y;
		}

		POINT center;
		center.x = sum_x / num_points;
		center.y = sum_y / num_points;

		return center;
	}
	void scaleFigure(const double scale) {
		POINT center = calculate_center(m_rb_vertices);
		for (int i = 0; i < m_rb_vertices.size(); i++) {
			// translate the point to the origin
			const double translated_x = m_rb_vertices[i].x - center.x;
			const double translated_y = m_rb_vertices[i].y - center.y;

			// scale the point
			const double scaled_x = translated_x * scale;
			const double scaled_y = translated_y * scale;

			// translate the point back to its original position
			m_rb_vertices[i].x = scaled_x + center.x;
			m_rb_vertices[i].y = scaled_y + center.y;
		}
	}
	void moveFigure(int x_shift, int y_shift) {
		for (auto& point_M : m_vertices) {
			point_M.x += x_shift;
			point_M.y += y_shift;
		}
		for (auto& point_B : m_rb_vertices) {
			point_B.x += x_shift;
			point_B.y += y_shift;
		}
	}
	void rotateFigure(float rotation)
	{
		// Конвертируем угол в радианы
		rotation *= 3.1415926535 / 180.0;

		// Проверяем, что фигура имеет хотя бы одну точку
		if (m_vertices.empty()) {
			return;
		}

		// Находим координаты первой точки
		const int centerX = calculate_center(m_vertices).x;
		const int centerY = calculate_center(m_vertices).y;

		// Вычисляем синус и косинус угла поворота
		const double cos_alpha = cos(rotation);
		const double sin_alpha = sin(rotation);

		// Проходим по всем точкам, начиная со второй, и вращаем их относительно первой точки
		for (size_t i = 0; i < m_vertices.size(); ++i) {
			m_rb_vertices[i].x = (m_vertices[i].x - centerX) * cos_alpha - (m_vertices[i].y - centerY) * sin_alpha + centerX;
			m_rb_vertices[i].y = (m_vertices[i].x - centerX) * sin_alpha + (m_vertices[i].y - centerY) * cos_alpha + centerY;
		}
	}
	void Fill(HDC hdc) const
	{
		// Создание закрашивающей кисти заданного цвета
		HBRUSH brush = CreateSolidBrush(m_color);

		// Сохранение текущего режима закраски
		int oldFillMode = GetPolyFillMode(hdc);

		// Установка нового режима закраски (закрашивание с помощью многоугольника)
		SetPolyFillMode(hdc, WINDING);

		// Создание массива точек, задающих многоугольник
		std::vector<POINT> points = m_rb_vertices;
		points.push_back(m_rb_vertices.front()); // Добавление первой точки в конец массива

		// Закрашивание многоугольника
		BeginPath(hdc);
		MoveToEx(hdc, points.back().x, points.back().y, nullptr);
		for (const auto& point : points)
		{
			LineTo(hdc, point.x, point.y);
		}
		EndPath(hdc);
		SelectObject(hdc, brush);
		FillPath(hdc);

		// Восстановление режима закраски
		SetPolyFillMode(hdc, oldFillMode);

		// Освобождение ресурсов
		DeleteObject(brush);
	}
	vector<POINT> m_vertices;  // вершины полигона
	vector<POINT> m_rb_vertices;  // вершины полигона (буфер)
	POINT center;
	POINT m_position;  // позиция полигона
	double m_rotation;  // угол поворота полигона (в радианах)
	double m_scale;  // масштаб полигона
	COLORREF m_color; // цвет полигона

public:
	
	void init(const std::vector<POINT>& vertices)
	{
		m_vertices = vertices; m_rb_vertices = vertices; m_position = { 0, 0 }; m_rotation = 0.0; m_scale = 1.0; m_color = RGB(0, 0, 0);
	}

	POINT GetCenter() {
		return calculate_center(m_vertices);
	}

	// Установка новой позиции полигона
	void SetColor(COLORREF color) { m_color = color; }

	// Установка новой позиции полигона
	void SetPosition(const POINT& position) {
		moveFigure(position.x, position.y);
	}

	// Установка нового угла поворота полигона
	void SetRotation(double rotation) {
		m_rotation += rotation; rotateFigure(m_rotation); scaleFigure(m_scale);
	}

	// Установка нового масштаба полигона
	void SetScale(double scale) {
		m_scale *= scale;
		rotateFigure(m_rotation);
		scaleFigure(m_scale);
	}

	// Отрисовка полигона в контексте устройства
	void Draw(HDC hdc) 
	{
		Fill(hdc);

		// Отрисовка полигона
		Polygon(hdc, m_rb_vertices.data(), m_rb_vertices.size());
	}

	void LGBT(HDC hdc, COLORREF startColor, COLORREF endColor, int durationMs)
	{
		// Вычисляем шаг изменения цвета
		const int steps = 100;
		const int stepDuration = durationMs / steps;

		// Вычисляем приращение компонент цвета
		const int rDelta = (GetRValue(endColor) - GetRValue(startColor)) / steps;
		const int gDelta = (GetGValue(endColor) - GetGValue(startColor)) / steps;
		const int bDelta = (GetBValue(endColor) - GetBValue(startColor)) / steps;

		// Начальные значения компонент цвета
		int r = GetRValue(startColor);
		int g = GetGValue(startColor);
		int b = GetBValue(startColor);

		// Изменяем цвет плавно в цикле
		for (int i = 0; i < steps; ++i)
		{
			SetColor(RGB(r, g, b));
			Fill(hdc);
			Polygon(hdc, m_rb_vertices.data(), m_rb_vertices.size());
			/*COLORREF color = RGB(r, g, b);
			Draw(hdc);
			HBRUSH brush = CreateSolidBrush(color);
			SelectObject(hdc, brush);
			Polygon(hdc, &m_rb_vertices[0], m_rb_vertices.size());
			DeleteObject(brush);*/
			r += rDelta;
			g += gDelta;
			b += bDelta;
			Sleep(stepDuration);
		}

		// Отрисовываем конечный цвет
		COLORREF color = RGB(GetRValue(endColor), GetGValue(endColor), GetBValue(endColor));
		Draw(hdc);
		HBRUSH brush = CreateSolidBrush(color);
		SelectObject(hdc, brush);
		Polygon(hdc, &m_rb_vertices[0], m_rb_vertices.size());
		DeleteObject(brush);
		SetColor(endColor);
	}
};