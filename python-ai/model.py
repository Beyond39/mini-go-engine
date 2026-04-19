from pathlib import Path 
import torch
import torch.nn as nn 
import torch.nn.functional as F

class SmallGoNet(nn.Module) :
    def __init__(self):
        super().__init__ ()

        self.backbone = nn.Sequential(
            nn.Conv2d(3, 32, kernel_size=3 , padding=1) ,
            nn.ReLU(inplace=True) ,

            nn.Conv2d(32, 64, kernel_size=3 , padding=1) ,
            nn.ReLU(inplace=True) ,

            nn.Conv2d(64, 64, kernel_size=3, padding=1) ,
            nn.ReLU(inplace=True) ,
        )

        self.policy_conv = nn.Conv2d(64, 2, kernel_size=1)
        self.policy_fc = nn.Linear(2 * 19 * 19 , 362) 

        self.value_conv = nn.Conv2d(64, 1, kernel_size=1)
        self.value_fc1 = nn.Linear(19 * 19, 64) 
        self.value_fc2 = nn.Linear(64, 1)

    def forward(self, x):
        feat = self.backbone(x) 

        p = self.policy_conv(feat) 
        p = p.view(p.size(0) , -1)
        p = self.policy_fc(p) 

        v = self.value_conv(feat) 
        v = v.view(v.size(0), -1)
        v = F.relu(self.value_fc1(v))
        v = torch.tanh(self.value_fc2(v))

        return p , v.squeeze(1) 
    
    