# Security Policy

## Supported Versions

Security updates are provided for the latest released version of genUp4win.

| Version | Supported          |
| ------- | ------------------ |
| Latest release | ✅ |
| Older releases | ❌ |

Users are encouraged to upgrade to the most recent version before reporting security issues.

## Reporting a Vulnerability

If you discover a security vulnerability in genUp4win, please report it responsibly.

### Preferred Disclosure Process

1. **Do not create a public GitHub issue** for security vulnerabilities.
2. Contact the maintainer privately with:
   - A description of the vulnerability
   - Steps to reproduce the issue
   - Potential impact
   - Any proposed mitigation or fix

### Contact

- GitHub: Open a private security advisory if available through GitHub Security Advisories.
- **Maintainer:** Stefan-Mihai MOGA
- **Repository:** [genUp4win](https://github.com/mihaimoga/genUp4win)

### What to Expect

After a report is received:

- The maintainer will review the report and assess its severity.
- Acknowledgment will be provided as soon as practical.
- Reasonable efforts will be made to validate and address legitimate vulnerabilities.
- If the report is accepted, a fix will be developed and released as soon as feasible.
- Public disclosure should be coordinated with the maintainer to allow users time to update.

## Security Best Practices

Because genUp4win downloads and installs software updates, developers integrating it into their applications should:

- Use HTTPS endpoints for update metadata and package downloads.
- Verify package integrity using checksums before installation.
- Host update files on trusted infrastructure.
- Protect update configuration files from unauthorized modification.
- Validate all downloaded content before execution.
- Regularly review dependencies and third-party components for known vulnerabilities.

## Scope

This policy applies to:

- The genUp4win source code
- Update checking functionality
- Download and installation mechanisms
- Release artifacts published by the project

Third-party libraries and external services should be reported to their respective maintainers when appropriate.

## Hall of Fame

Contributors who responsibly disclose security vulnerabilities may be acknowledged in release notes or project documentation, subject to their preference.
