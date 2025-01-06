#include <windows.h>
#include <vector>
#include <iostream>
#include <string>
#include <sstream>
#include <iomanip>
#include <Eigen/Dense>
#include <cctype>
#include <stdexcept>
#include <exception>
#include <locale>
#include <codecvt>
#include <fstream>

using namespace std;
using namespace Eigen;

const double EPSILON = 1e-10; // Точность

// Глобальные переменные для управления элементами интерфейса
HWND hEditMatrixA, hEditMatrixB, hEditResult;

// Класс для вычислений собственных значений и векторов
class GeneralizedEigenCalculator {
public:
    GeneralizedEigenCalculator() = default;

    // Метод для вычисления собственных значений и векторов
    wstring calculate(const vector<double>& valuesA, const vector<double>& valuesB, wstring& errorMessage) {
        int n = static_cast<int>(sqrt(valuesA.size()));

        // Проверка на корректность размеров матриц
        if (n * n != valuesA.size() || n * n != valuesB.size()) {
            errorMessage += L"Матрицы должны быть квадратными и одинакового размера.\n";
            return L"";
        }

        // Шаг 1: Заполнение матрицы A
        MatrixXd A(n, n);
        for (int i = 0; i < n; ++i) {
            for (int j = 0; j < n; ++j) {
                A(i, j) = valuesA[i * n + j];
            }
        }

        // Шаг 2: Заполнение матрицы B
        MatrixXd B(n, n);
        for (int i = 0; i < n; ++i) {
            for (int j = 0; j < n; ++j) {
                B(i, j) = valuesB[i * n + j];
            }
        }

        // Шаг 3: Вычисление обобщенных собственных значений
        GeneralizedEigenSolver<MatrixXd> ges;
        ges.compute(A, B, true);

        // Проверка на успешность вычислений
        if (ges.info() != Success) {
            errorMessage += L"Ошибка при вычислении обобщенных собственных значений.\n";
            return L"";
        }

        // Извлечение обобщенных собственных значений и векторов
        VectorXd Lambda = ges.eigenvalues().real(); // Получаем действительные значения
        MatrixXd Phi = ges.eigenvectors().real(); // Получаем действительные векторы

        // Формируем вывод
        wostringstream resultStream;
        resultStream << fixed << setprecision(15); // Увеличиваем точность вывода

        for (int i = 0; i < n; ++i) {
            resultStream << L"Собственное значение: " << Lambda[i] << L"\r\n";
        }

        return resultStream.str();
    }
};

// Функция для проверки, является ли строка числом
bool isNumber(const wstring& str) {
    if (str.empty()) return false;
    wchar_t* end;
    wcstod(str.c_str(), &end);
    return end != str.c_str() && *end == L'\0'; // Проверка на успешное преобразование
}

// Функция для проверки, является ли количество элементов квадратом целого числа
bool isPerfectSquare(int number) {
    int root = static_cast<int>(sqrt(number));
    return root * root == number;
}

// Процедура обработки сообщений
LRESULT CALLBACK SoftwareMainProcedure(HWND hWnd, UINT msg, WPARAM wp, LPARAM lp) {
    static GeneralizedEigenCalculator calculator; // Создаем экземпляр класса

    switch (msg) {
    case WM_CREATE: {
        hEditMatrixA = CreateWindowW(L"EDIT", L"", WS_CHILD | WS_VISIBLE | WS_BORDER | ES_MULTILINE | ES_AUTOVSCROLL | WS_VSCROLL,
            10, 10, 500, 200, hWnd, NULL, NULL, NULL);

        hEditMatrixB = CreateWindowW(L"EDIT", L"", WS_CHILD | WS_VISIBLE | WS_BORDER | ES_MULTILINE | ES_AUTOVSCROLL | WS_VSCROLL,
            10, 220, 500, 200, hWnd, NULL, NULL, NULL);

        CreateWindowW(L"BUTTON", L"Вычислить", WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON,
            10, 430, 100, 30, hWnd, (HMENU)1, NULL, NULL);

        CreateWindowW(L"BUTTON", L"Вычислить из файла", WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON,
            120, 430, 150, 30, hWnd, (HMENU)2, NULL, NULL);

        hEditResult = CreateWindowW(L"EDIT", L"", WS_CHILD | WS_VISIBLE | WS_BORDER | ES_MULTILINE | ES_READONLY | ES_AUTOVSCROLL | WS_VSCROLL,
            10, 470, 500, 200, hWnd, NULL, NULL, NULL);
        break;
    }
    case WM_COMMAND: {
        if (LOWORD(wp) == 1) { // Логика для кнопки "Вычислить"
            try {
                int lengthA = GetWindowTextLength(hEditMatrixA);
                int lengthB = GetWindowTextLength(hEditMatrixB);
                if (lengthA == 0 || lengthB == 0) {
                    MessageBox(hWnd, L"Поля для ввода матриц не могут быть пустыми.", L"Ошибка", MB_OK | MB_ICONERROR);
                    SetWindowText(hEditResult, L"");
                    return 0;
                }

                vector<wchar_t> bufferA(lengthA + 1);
                vector<wchar_t> bufferB(lengthB + 1);

                GetWindowText(hEditMatrixA, bufferA.data(), lengthA + 1);
                GetWindowText(hEditMatrixB, bufferB.data(), lengthB + 1);

                // Разбор значений для матрицы A
                vector<double> valuesA;
                wstringstream ss1(bufferA.data());
                wstring token1;

                while (ss1 >> token1) {
                    if (!isNumber(token1)) {
                        MessageBox(hWnd, L"Ввод только чисел!", L"Ошибка", MB_OK | MB_ICONERROR);
                        return 0;
                    }
                    valuesA.push_back(wcstod(token1.c_str(), nullptr));
                }

                // Разбор значений для матрицы B
                vector<double> valuesB;
                wstringstream ss2(bufferB.data());
                wstring token2;

                while (ss2 >> token2) {
                    if (!isNumber(token2)) {
                        MessageBox(hWnd, L"Ввод только чисел!", L"Ошибка", MB_OK | MB_ICONERROR);
                        return 0;
                    }
                    valuesB.push_back(wcstod(token2.c_str(), nullptr));
                }

                if (valuesA.empty() || valuesB.empty()) {
                    MessageBox(hWnd, L"Матрицы не могут быть пустыми.", L"Ошибка", MB_OK | MB_ICONERROR);
                    return 0;
                }

                int sizeA = valuesA.size();
                int sizeB = valuesB.size();

                if (!isPerfectSquare(sizeA) || !isPerfectSquare(sizeB)) {
                    MessageBox(hWnd, L"Размеры матриц должны быть квадратами целых чисел.", L"Ошибка", MB_OK | MB_ICONERROR);
                    return 0;
                }

                int nA = static_cast<int>(sqrt(sizeA));
                int nB = static_cast<int>(sqrt(sizeB));

                if (nA != nB) {
                    MessageBox(hWnd, L"Матрицы должны быть одинакового размера.", L"Ошибка", MB_OK | MB_ICONERROR);
                    return 0;
                }

                wstring errorMessage;
                // Вычисляем собственные значения и векторы через класс
                wstring result = calculator.calculate(valuesA, valuesB, errorMessage);

                if (!errorMessage.empty()) {
                    MessageBox(hWnd, errorMessage.c_str(), L"Ошибка", MB_OK | MB_ICONERROR);
                    SetWindowText(hEditResult, L"");
                }
                else {
                    SetWindowText(hEditResult, result.c_str());
                }
            }
            catch (...) {
                MessageBox(hWnd, L"Произошла неизвестная ошибка.", L"Ошибка", MB_OK | MB_ICONERROR);
            }
        }
        else if (LOWORD(wp) == 2) { // Логика для кнопки "Вычислить из файла"
            try {
                wstring filePathA = L"matrixA.txt"; // Путь к первому файлу
                wstring filePathB = L"matrixB.txt"; // Путь ко второму файлу

                // Функция для чтения матрицы из файла
                auto readMatrixFromFile = [](const wstring& filePath, vector<double>& values) -> bool {
                    wifstream file(filePath);
                    if (!file.is_open()) {
                        return false;
                    }
                    wstring line;
                    while (getline(file, line)) {
                        wstringstream ss(line);
                        wstring token;
                        while (ss >> token) {
                            if (!isNumber(token)) {
                                return false;
                            }
                            values.push_back(wcstod(token.c_str(), nullptr));
                        }
                    }
                    return true;
                    };

                vector<double> valuesA, valuesB;
                if (!readMatrixFromFile(filePathA, valuesA) || !readMatrixFromFile(filePathB, valuesB)) {
                    MessageBox(hWnd, L"Ошибка при чтении файлов или неверный формат данных.", L"Ошибка", MB_OK | MB_ICONERROR);
                    return 0;
                }

                if (valuesA.empty() || valuesB.empty()) {
                    MessageBox(hWnd, L"Файлы матриц не могут быть пустыми.", L"Ошибка", MB_OK | MB_ICONERROR);
                    return 0;
                }

                int sizeA = valuesA.size();
                int sizeB = valuesB.size();

                if (!isPerfectSquare(sizeA) || !isPerfectSquare(sizeB)) {
                    MessageBox(hWnd, L"Размеры матриц в файлах должны быть квадратами целых чисел.", L"Ошибка", MB_OK | MB_ICONERROR);
                    return 0;
                }

                int nA = static_cast<int>(sqrt(sizeA));
                int nB = static_cast<int>(sqrt(sizeB));

                if (nA != nB) {
                    MessageBox(hWnd, L"Матрицы из файлов должны быть одинакового размера.", L"Ошибка", MB_OK | MB_ICONERROR);
                    return 0;
                }

                wstring errorMessage;
                wstring result = calculator.calculate(valuesA, valuesB, errorMessage);

                if (!errorMessage.empty()) {
                    MessageBox(hWnd, errorMessage.c_str(), L"Ошибка", MB_OK | MB_ICONERROR);
                    SetWindowText(hEditResult, L"");
                }
                else {
                    SetWindowText(hEditResult, result.c_str());
                }

            }
            catch (...) {
                MessageBox(hWnd, L"Произошла неизвестная ошибка при обработке файлов.", L"Ошибка", MB_OK | MB_ICONERROR);
            }
        }
        break;
    }
    case WM_DESTROY:
        PostQuitMessage(0);
        break;
    default:
        return DefWindowProc(hWnd, msg, wp, lp);
    }
    return 0;
}

int WINAPI WinMain(HINSTANCE hInst, HINSTANCE hPrevInst, LPSTR args, int ncmdshow) {
    WNDCLASS SoftwareMainClass = { 0 };
    SoftwareMainClass.hInstance = hInst;
    SoftwareMainClass.lpszClassName = L"GeneralizedEigenvalueCalculator";
    SoftwareMainClass.lpfnWndProc = SoftwareMainProcedure;
    SoftwareMainClass.hCursor = LoadCursor(NULL, IDC_ARROW);
    SoftwareMainClass.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
    RegisterClass(&SoftwareMainClass);

    HWND hWnd = CreateWindow(L"GeneralizedEigenvalueCalculator", L"Калькулятор обобщенных собственных значений",
        WS_OVERLAPPEDWINDOW | WS_VISIBLE, CW_USEDEFAULT, CW_USEDEFAULT, 550, 700,
        NULL, NULL, hInst, NULL);

    MSG msg;
    while (GetMessage(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    return 0;
}
