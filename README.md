# OBC-firmware

This repository holds all the code written by UW Orbital's software team. This includes OBC firmware/embedded software, ground station software, and all testing infrastructure.

## Table of Contents

- [UW Orbital Docs](#uw-orbital-docs)
- [Notion](#notion)
- [Firmware Dependencies](#firmware-dependencies)
- [Contributing](#contributing)
- [Authors](#authors)

## UW Orbital Docs
The UW Orbital Docs are a great source of codebase documentation. The site includes many detailed setup guides as well as code style guides. **We highly recommend following these guides if you are new to the team or software development in general.**

You will find it most helpful to check out the setup guides, build tutorials, and style guides.

Check out the site here: https://uworbital.github.io/docs/

The documentation site now contains most of the information previously found in this README.

## Notion
The team Notion is another important source of information. Here you'll find other team documentation, onboarding info, and links to useful resources.

The software section of the Notion is available here: https://www.notion.so/uworbital/Software-a613c4aaa36449969010cabba2fc329d

Useful links:
- Firmware Onboarding: https://www.notion.so/uworbital/Firmware-Onboarding-48aeba74f55045f5929b40d029202842
- Ground Station Onboarding: https://www.notion.so/uworbital/Ground-Station-Onboarding-10f8a26d767780d7ae8de921d9782b77
- Software Development Life Cycle (SDLC): https://www.notion.so/uworbital/Software-Development-Life-Cycle-1218a26d767780619d42fa22f0785e73

## Firmware Dependencies

#### HALCoGen

Download HALCoGen here: https://www.ti.com/tool/HALCOGEN#downloads. This will be used for configuring the HAL. Unfortunately, the tool is only available on Windows. If you're on a non-Windows system, you may be able to set HALCoGen up in a Windows VM or someone else on the team can make HAL changes for you. We don't have to modify the HAL very often anyways.

#### Code Composer Studio (CCS)

Download Code Composer Studio (CCS): https://www.ti.com/tool/CCSTUDIO. This will be used for debugging.

#### Uniflash

Download UniFlash here: https://www.ti.com/tool/UNIFLASH#downloads. This will be used for flashing the RM46.

## Contributing

1. Make sure you're added as a member to the UW Orbital organization on GitHub.
2. Create a feature branch for whatever task you're working on.
   - Our branch naming scheme is `<developer_name>/<feature_description>`.
     - Example: `danielg/implement-random-device-driver`
3. Make a PR.
   - For the PR description, make sure to fill in all the required details in the generated template.
   - Add at least three PR reviewers, including one software lead. When a PR is created, PR stats are added as a comment. You can use these stats to choose reviewers. Send a message in the #pr channel on Discord to notify the reviewers of your PR.
4. Make any requested changes and merge your branch onto main once the PR is approved.

## Authors

This codebase was developed by the members of UW Orbital, the University of Waterloo's CubeSat design team.
