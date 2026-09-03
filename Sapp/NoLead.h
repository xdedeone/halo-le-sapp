#pragma once

extern bool Lead[16];
extern size_t CustomLead[16];

extern size_t PingIndex[16];

void UpdateHistory();

void ResetHistory(size_t PlayerIndex);
void ClearNoLeadCache();

void InstallNoLeadHandler();
void UninstallNoLeadHandler();